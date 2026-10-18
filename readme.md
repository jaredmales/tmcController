A c++ class to interface to Thorlabs Motion Control devices using libftdi1.

Note: this is only used by me for KPZ101 k-cubes, but it should be straightforward to extend for other devices.  PRs welcome.

See tmcController for a detailed description of the API.

See demo.cpp for an example of usage.

Browse documentation at https://jaredmales.github.io/tmcController-docs/
## Connection recovery

`TMCC_IO_RECOVERY_VERSION` is defined as `1` for the bounded-I/O API. A controller
uniquely owns its FTDI context and cannot be copied. Repeated `open()`/`close()`
is safe; changing device identity closes the previous device. Failed `connect()`
releases the handle and clears both connection flags, including after cancellation.

`ioTimeout(milliseconds)` sets the overall reply deadline and individual USB
operation timeout (default 1000 ms, clamped to 1..60000). Reply reads use slices of
at most 100 ms and settling waits check cancellation every 10 ms. A USB operation
already in progress may take its configured timeout to return.

Applications may override `communicationAllowed() const` to cancel work using a
thread-safe, nonblocking power/shutdown snapshot. The default permits communication.
The caller must still serialize all controller access, including close/destruction;
the hook allows cancellation without concurrently closing an active USB handle.

In addition to existing FTDI error offsets, operations return:

| Result | Meaning |
| --- | --- |
| -300 | Overall reply deadline expired, including repeated zero-byte reads |
| -301 | Short write; the command is not replayed |
| -302 | Unexpected reply ID, address, length or channel |
| -400 plus FTDI result | Command flush failed |
| -666 | Communication canceled or device unavailable |

After a transport or framing error, the caller should close/reconnect and reinitialize
its device state. The library never replays a partially written command. The wire
implementation validates replies for the supported KPZ101 messages; other devices
or firmware should be checked before extending this behavior.

## Offline tests

The tests require C++17 and libftdi1/libusb development headers. They supply fake
FTDI functions and never link libftdi or access USB hardware. The reusable boundary
in `tests/ftdiHarness.hpp` also supports downstream application regression tests.

```sh
make -C tests check
```

For nonstandard headers, set `FTDI_CFLAGS`. To run AddressSanitizer and
UndefinedBehaviorSanitizer, including the fast-math nonfinite-input regression:

```sh
make -C tests clean
make -C tests check \
  CXXFLAGS='-std=c++17 -O1 -g -Wall -Wextra -Werror -ffast-math -fsanitize=address,undefined -fno-omit-frame-pointer'
```

On 2026-10-17, 154 checks passed on a workstation. LeakSanitizer could not run
under that environment's tracing setup; setting `ASAN_OPTIONS=detect_leaks=0`
allowed the ASan/UBSan run. Context ownership is also checked explicitly by the
fake FTDI boundary. These tests do not establish hardware or firmware compatibility.
