/** \file ftdiHarness.hpp
 * \brief Scripted FTDI boundary for hardware-free controller and application tests.
 */
#ifndef tmcController_tests_ftdiHarness_hpp
#define tmcController_tests_ftdiHarness_hpp

#include <ftdi.h>
#include <algorithm>
#include <cstring>
#include <deque>
#include <functional>
#include <map>
#include <string>
#include <vector>

/// \cond DOXYGEN_SUPPRESS_TEST_HARNESS
namespace ftdiHarness
{
/// One actual dependency call crossing the FTDI boundary.
struct Call
{
    /// Context's configured serial number, empty until opened.
    std::string serial;
    /// FTDI operation name.
    std::string operation;
    /// APT command ID, when writing or reading a reply.
    unsigned command{0};
    /// Actual bytes passed to a write.
    std::vector<unsigned char> bytes;
};

/// Owned state for one fake context; no real USB handle exists.
struct Context
{
    /// Configured device serial number.
    std::string serial;
    /// Pending reply bytes, generated from actual command traffic.
    std::deque<unsigned char> reply;
    /// Last APT command written.
    unsigned command{0};
};

/// Resettable script shared by one sequential test fixture.
struct Script
{
    /// Context ownership and reply queues.
    std::map<ftdi_context *, Context> contexts;
    /// Recorded boundary calls.
    std::vector<Call> calls;
    /// Operation selected to fail; empty means none.
    std::string failure;
    /// Serial selected to fail; empty matches either axis.
    std::string serial;
    /// Matching operation invocation to fail.
    unsigned failureCall{1};
    /// Matching invocations so far.
    unsigned matches{0};
    /// Failure result, including short positive writes or zero-byte reads.
    int result{-1};
    /// Apply the failure on every matching call at or after failureCall.
    bool persistent{false};
    /// Fail context allocation.
    bool allocationFailure{false};
    /// Detect opening an occupied context or I/O on a closed context.
    bool invalidAccess{false};
    /// Maximum bytes returned per read; zero produces endless empty reads.
    unsigned readChunk{256};
    /// Modify a generated reply before the actual decoder reads it.
    std::function<void(std::vector<unsigned char> &)> replyHook;
    /// Deliver target changes or coordinate a thread at a precise I/O boundary.
    std::function<void(const Call &)> beforeIo;
};

/// Current sequential fixture's hardware-free script.
inline Script script;

/// Record a call and return a selected failure, or the supplied success result.
inline int call( ftdi_context *context /**< [in] Fake context. */,
                 const std::string &operation /**< [in] Boundary operation. */,
                 int success = 0 /**< [in] Normal result. */,
                 const unsigned char *bytes = nullptr /**< [in] Optional written bytes. */,
                 int size = 0 /**< [in] Byte count. */ )
{
    auto &device = script.contexts.at(context);
    Call event{device.serial, operation, device.command, {}};
    if( bytes ) event.bytes.assign(bytes, bytes+size);
    script.calls.push_back(event);
    if( operation != "open" && operation != "close" && !context->usb_dev ) script.invalidAccess = true;
    if( script.beforeIo ) script.beforeIo(event);
    if( operation == script.failure && (script.serial.empty() || script.serial == device.serial) )
    {
        ++script.matches;
        if( script.matches == script.failureCall || (script.persistent && script.matches >= script.failureCall) )
            return script.result;
    }
    return success;
}
} // namespace ftdiHarness

extern "C"
{
/// Allocate a fake context with no USB resources.
inline ftdi_context *ftdi_new()
{
    if( ftdiHarness::script.allocationFailure ) return nullptr;
    auto *context = new ftdi_context{};
    ftdiHarness::script.contexts.emplace(context, ftdiHarness::Context{});
    return context;
}

/// Free only the fake context.
inline void ftdi_free( ftdi_context *context /**< [in] Owned fake context. */ )
{
    if( context && context->usb_dev ) ftdiHarness::script.invalidAccess = true;
    ftdiHarness::script.contexts.erase(context);
    delete context;
}

/// Simulate a device open and detect attempts to overwrite an owned handle.
inline int ftdi_usb_open_desc_index( ftdi_context *context /**< [in] Context. */,
                                     [[maybe_unused]] int vendor /**< [in] Ignored vendor ID. */,
                                     [[maybe_unused]] int product /**< [in] Ignored product ID. */,
                                     [[maybe_unused]] const char *description /**< [in] Ignored description. */,
                                     const char *serial /**< [in] Device identity. */,
                                     [[maybe_unused]] unsigned index /**< [in] Ignored match index. */ )
{
    if( context->usb_dev ) ftdiHarness::script.invalidAccess = true;
    ftdiHarness::script.contexts.at(context).serial = serial;
    int result = ftdiHarness::call(context, "open");
    if( result == 0 ) context->usb_dev = reinterpret_cast<libusb_device_handle *>(context);
    return result;
}

/// Model libftdi cleanup: even an interface-release error clears the handle.
inline int ftdi_usb_close( ftdi_context *context /**< [in] Context to release. */ )
{
    int result = ftdiHarness::call(context, "close");
    context->usb_dev = nullptr;
    ftdiHarness::script.contexts.at(context).reply.clear();
    return result;
}

/// Script chip-ID acquisition.
inline int ftdi_read_chipid( ftdi_context *context /**< [in] Context. */, unsigned int *id /**< [out] Fake ID. */ )
{
    *id = 123;
    return ftdiHarness::call(context, "chip");
}

/// Script baud-rate setup.
inline int ftdi_set_baudrate( ftdi_context *context /**< [in] Context. */, [[maybe_unused]] int baud /**< [in] Ignored baud rate. */ )
{
    return ftdiHarness::call(context, "baud");
}

/// Script line-property setup.
inline int ftdi_set_line_property( ftdi_context *context /**< [in] Context. */,
                                   [[maybe_unused]] ftdi_bits_type bits /**< [in] Ignored word length. */,
                                   [[maybe_unused]] ftdi_stopbits_type stop /**< [in] Ignored stop-bit selection. */,
                                   [[maybe_unused]] ftdi_parity_type parity /**< [in] Ignored parity selection. */ )
{
    return ftdiHarness::call(context, "line");
}

/// Script an input/output flush.
inline int ftdi_tcioflush( ftdi_context *context /**< [in] Context. */ )
{
    ftdiHarness::script.contexts.at(context).reply.clear();
    return ftdiHarness::call(context, "flush");
}

/// Script USB reset.
inline int ftdi_usb_reset( ftdi_context *context /**< [in] Context. */ )
{
    return ftdiHarness::call(context, "reset");
}

/// Script flow-control setup.
inline int ftdi_setflowctrl( ftdi_context *context /**< [in] Context. */, [[maybe_unused]] int flow /**< [in] Ignored flow mode. */ )
{
    return ftdiHarness::call(context, "flow");
}

/// Script RTS setup.
inline int ftdi_setrts( ftdi_context *context /**< [in] Context. */, [[maybe_unused]] int state /**< [in] Ignored RTS state. */ )
{
    return ftdiHarness::call(context, "rts");
}

/// Generate valid APT replies from the real controller's wire commands.
inline int ftdi_write_data( ftdi_context *context /**< [in] Context. */,
                            const unsigned char *bytes /**< [in] Written bytes. */,
                            int size /**< [in] Written count. */ )
{
    auto &device = ftdiHarness::script.contexts.at(context);
    device.command = bytes[0] | (unsigned(bytes[1]) << 8);
    int result = ftdiHarness::call(context, "write", size, bytes, size);
    if( result != size ) return result;
    int length = 0;
    switch( device.command )
    {
        case 0x0005: length=90; break;
        case 0x0211: length=6; break;
        case 0x0644: length=10; break;
        case 0x0660: length=16; break;
        case 0x07D2: length=8; break;
        case 0x07D5: length=16; break;
        case 0x07F1: length=40; break;
    }
    if( length )
    {
        std::vector<unsigned char> reply(length, 0);
        reply[0] = (device.command+1) & 255;
        reply[1] = (device.command+1) >> 8;
        reply[2] = length > 6 ? length-6 : 1;
        reply[3] = length > 6 ? 0 : 1;
        reply[4] = length > 6 ? 0x81 : 0x01;
        reply[5] = 0x50;
        if( length > 6 ) reply[6] = 1;
        if( device.command == 0x07D5 ) reply[8] = 3;
        if( device.command == 0x0644 ) { reply[8] = 0x00; reply[9] = 0x40; }
        if( ftdiHarness::script.replyHook ) ftdiHarness::script.replyHook(reply);
        device.reply.insert(device.reply.end(), reply.begin(), reply.end());
    }
    return size;
}

/// Return a scripted fragment or failure without waiting for hardware.
inline int ftdi_read_data( ftdi_context *context /**< [in] Context. */,
                           unsigned char *bytes /**< [out] Received bytes. */,
                           int size /**< [in] Capacity. */ )
{
    auto &reply = ftdiHarness::script.contexts.at(context).reply;
    int count = std::min({size, int(reply.size()), int(ftdiHarness::script.readChunk)});
    int result = ftdiHarness::call(context, "read", count);
    if( result < 0 ) return result;
    count = std::min(count, result);
    for( int n=0; n<count; ++n ) { bytes[n]=reply.front(); reply.pop_front(); }
    return count;
}

/// Supply a stable fake error without inspecting a real context.
inline const char *ftdi_get_error_string( [[maybe_unused]] ftdi_context *context /**< [in] Ignored context. */ )
{
    return "scripted FTDI error";
}
}
/// \endcond
#endif
