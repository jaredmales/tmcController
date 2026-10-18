/** \file tmcController_test.cpp
 * \brief Standalone offline regression tests for connection ownership and bounded APT I/O.
 */
#include "ftdiHarness.hpp"
#include "../tmcController.hpp"
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <type_traits>

/// \cond DOXYGEN_SUPPRESS_TEST_HARNESS
namespace
{
/// Number of verified conditions.
unsigned checks = 0;

/// Fail immediately with the expression and source line.
void check( bool result /**< [in] Condition. */, const char *expression /**< [in] Condition text. */,
            unsigned line /**< [in] Source line. */ )
{
    ++checks;
    if( !result )
    {
        std::cerr << "check failed at " << line << ": " << expression << '\n';
        std::exit(1);
    }
}
#define CHECK(expression) check( (expression), #expression, __LINE__ )

/// Controller with thread-safe cancellation and captured error messages.
class Controller : public tmcController
{
public:
    /// Permission supplied by the owning application.
    std::atomic<bool> allowed{true};
    /// Last protocol or transport diagnostic.
    std::string error;
    /// Use zero test delays without changing production defaults.
    Controller();
    /// Read the test's atomic cancellation flag.
    bool communicationAllowed() const override;
    /// Capture an FTDI failure.
    void ftdiErrmsg( const std::string &source /**< [in] Operation. */,
                     const std::string &message /**< [in] Error. */,
                     int result /**< [in] Result. */,
                     const std::string &file /**< [in] Source file. */,
                     int line /**< [in] Source line. */ ) override;
    /// Capture a protocol failure.
    void otherErrmsg( const std::string &source /**< [in] Operation. */,
                      const std::string &message /**< [in] Error. */,
                      const std::string &file /**< [in] Source file. */,
                      int line /**< [in] Source line. */ ) override;
};
Controller::Controller()
{
    serial("test");
    preFlushSleep(0);
    postFlushSleep(0);
    postChanEnableSleep(0);
}
bool Controller::communicationAllowed() const
{
    return allowed.load();
}
void Controller::ftdiErrmsg( const std::string &, const std::string &message, int, const std::string &, int )
{
    error = message;
}
void Controller::otherErrmsg( const std::string &, const std::string &message, const std::string &, int )
{
    error = message;
}

/// Count operations recorded by the FTDI boundary.
unsigned count( const std::string &operation /**< [in] Operation name. */ )
{
    const auto &calls = ftdiHarness::script.calls;
    return std::count_if(calls.begin(), calls.end(), [&](const auto &call){return call.operation == operation;});
}

/// Check ownership, idempotent close, and failed reconnect cleanup.
void lifecycle()
{
    using ftdiHarness::script;
    static_assert(!std::is_copy_constructible<tmcController>::value, "context must not be copied");
    static_assert(!std::is_copy_assignable<tmcController>::value, "context must not be assigned");
    script = {};
    {
        Controller controller;
        CHECK(controller.open() == 0);
        CHECK(controller.open() == 0);
        CHECK(count("open") == 1);
        controller.serial("replacement");
        CHECK(!controller.opened());
        CHECK(controller.open() == 0);
        CHECK(script.contexts.at(const_cast<ftdi_context *>(controller.ftdi())).serial == "replacement");
        CHECK(count("close") == 1);
        CHECK(controller.connect() == 0);
        script.failure = "close";
        CHECK(controller.close() < 0);
        CHECK(!controller.opened());
        CHECK(!controller.connected());
        CHECK(controller.ftdi()->usb_dev == nullptr);
        CHECK(controller.close() == 0);
        CHECK(count("close") == 2);
        script.failure.clear();
        CHECK(controller.connect() == 0);
    }
    CHECK(script.contexts.empty());
    CHECK(!script.invalidAccess);
    for( const auto &operation : {"open", "chip", "baud", "line", "flush", "reset", "flow", "rts"} )
    {
        script = {};
        {
            Controller controller;
            CHECK(controller.connect() == 0);
            if(std::string(operation) == "open") CHECK(controller.close() == 0);
            script.failure = operation;
            CHECK(controller.connect() < 0);
            CHECK(!controller.opened());
            CHECK(!controller.connected());
            CHECK(controller.ftdi()->usb_dev == nullptr);
            script.failure.clear();
            CHECK(controller.connect() == 0);
        }
        CHECK(script.contexts.empty());
        CHECK(!script.invalidAccess);
    }
    script = {};
    script.allocationFailure = true;
    { Controller controller; CHECK(controller.open() < 0); CHECK(controller.close() == 0); }
    CHECK(script.contexts.empty());
}

/// Verify fragmented replies, all implemented messages, headers, timeout, and short-transfer errors.
void protocol()
{
    using ftdiHarness::script;
    script = {};
    {
        Controller controller;
        CHECK(controller.connect() == 0);
        script.readChunk = 1;
        tmcController::HWInfo hardware;
        tmcController::KMMIParams mmi;
        tmcController::TPZIOSettings settings;
        tmcController::PZStatus status;
        tmcController::EnableState enabled;
        float volts = 0;
        uint16_t display = 0;
        CHECK(controller.hw_req_info(hardware) == 0);
        CHECK(controller.kpz_req_kcubemmiparams(mmi) == 0);
        CHECK(controller.kpz_set_kcubemmiparams(mmi) == 0);
        CHECK(controller.pz_req_tpz_iosettings(settings) == 0);
        CHECK(controller.pz_set_tpz_iosettings(settings) == 0);
        CHECK(controller.pz_req_pzstatusupdate(status) == 0);
        CHECK(controller.mod_req_chanenablestate(enabled, 1) == 0);
        CHECK(controller.pz_req_outputvolts(volts) == 0);
        CHECK(volts > 0 && volts < 1);
        CHECK(controller.pz_set_outputvolts(0.5) == 0);
        auto writes = count("write");
        CHECK(controller.pz_set_outputvolts(std::numeric_limits<float>::quiet_NaN()) < 0);
        CHECK(controller.pz_set_outputvolts(std::numeric_limits<float>::infinity()) < 0);
        CHECK(controller.pz_set_outputvolts(-std::numeric_limits<float>::infinity()) < 0);
        CHECK(controller.pz_set_outputvolts(1.1f) < 0);
        CHECK(count("write") == writes);
        CHECK(controller.mod_identify() == 0);
        CHECK(controller.hw_stop_updatemsgs() == 0);
        CHECK(controller.mod_set_chanenablestate(1, tmcController::EnableState::disabled) == 0);
        CHECK(controller.pz_req_tpz_dispsettings(display) == 0);
        CHECK(controller.pz_set_tpz_dispsettings(12) == 0);
        script.failure = "flush";
        CHECK(controller.pz_set_outputvolts(0.5) < 0);
        script.failure = "write";
        script.matches = 0;
        script.result = 3;
        CHECK(controller.pz_req_outputvolts(volts) == -301);
        script.failure.clear();
        script.readChunk = 256;
        for( unsigned byte : {0u, 2u, 4u, 5u, 6u} )
        {
            script.replyHook = [=](auto &reply){reply.at(byte) ^= 1;};
            CHECK(controller.pz_req_outputvolts(volts) == -302);
        }
        script.replyHook = {};
        script.readChunk = 0;
        controller.ioTimeout(5);
        auto start = std::chrono::steady_clock::now();
        CHECK(controller.pz_req_outputvolts(volts) == -300);
        CHECK(std::chrono::steady_clock::now()-start < std::chrono::seconds(1));
        CHECK(count("read") < 1000);
    }
    CHECK(!script.invalidAccess);
    CHECK(script.contexts.empty());
}

/// Cancel at every connection boundary, during a read, and during channel settling.
void cancellation()
{
    using ftdiHarness::script;
    for(const auto &operation : {"open", "chip", "baud", "line", "flush", "reset", "flow", "rts"})
    {
        script = {};
        {
            Controller controller;
            script.beforeIo = [&](const auto &call){if(call.operation == operation) controller.allowed = false;};
            CHECK(controller.connect() == -666);
            CHECK(!controller.opened());
            CHECK(!controller.connected());
            script.beforeIo = {};
        }
        CHECK(!script.invalidAccess);
    }
    script = {};
    {
        Controller controller;
        CHECK(controller.connect() == 0);
        float volts;
        script.beforeIo = [&](const auto &call){if(call.operation == "read") controller.allowed = false;};
        CHECK(controller.pz_req_outputvolts(volts) == -666);
        controller.allowed = true;
        script.beforeIo = [&](const auto &call){if(call.operation == "write") controller.allowed = false;};
        CHECK(controller.mod_set_chanenablestate(1, tmcController::EnableState::enabled) == -666);
        script.beforeIo = {};
    }
    CHECK(!script.invalidAccess);
}
} // namespace
/// \endcond

/// Run all hardware-free dependency regressions.
int main()
{
    lifecycle();
    protocol();
    cancellation();
    std::cout << checks << " checks passed\n";
}
