# Netforge

Lightweight asynchronous TCP networking library for modern C++.

Netforge provides a simple object-oriented API for building asynchronous TCP servers on top of Boost.Asio. It handles accepting connections, session lifetime, asynchronous reads and writes, write queues, and safe access to session state.

## Features

- Asynchronous TCP server
- Object-oriented session architecture
- `Server<Session>` template
- `TcpSession` with asynchronous read/write
- Thread-safe writes through Asio strands
- Automatic session lifetime management
- Per-session outgoing message queue
- Broadcast messages to all connected sessions
- Connection lifecycle callbacks
- Error handling callbacks
- C++20
- CMake-based build system
- Debug and Release configurations

## Requirements

- C++20 compatible compiler
- CMake 3.20 or newer
- Boost
- Git

Netforge uses Boost.Asio for asynchronous TCP networking.

Boost is **not downloaded automatically by CMake**. It must be installed separately.

### Supported compilers

Netforge is intended to work with modern:

- MSVC
- GCC
- Clang

## Project structure

```text
Netforge/
├── CMakeLists.txt
├── include/
│   └── netforge/
│       ├── Server.hpp
│       ├── Session.hpp
│       └── TcpSession.hpp
├── src/
│   ├── Session.cpp
│   └── TcpSession.cpp
├── examples/
│   └── echo/
│       └── Main.cpp
└── README.md
```

## Quick start

Clone the repository:

```bash
git clone https://github.com/GiperB0la/Netforge.git
cd Netforge
```

### Linux

Install the required dependencies.

On Debian/Ubuntu:

```bash
sudo apt update
sudo apt install build-essential cmake libboost-all-dev
```

Configure a Debug build:

```bash
cmake -S . -B build/linux-debug \
    -DCMAKE_BUILD_TYPE=Debug \
    -DNETFORGE_BUILD_EXAMPLES=ON \
    -DNETFORGE_BUILD_TESTS=OFF
```

Build:

```bash
cmake --build build/linux-debug --parallel
```

For a Release build:

```bash
cmake -S . -B build/linux-release \
    -DCMAKE_BUILD_TYPE=Release \
    -DNETFORGE_BUILD_EXAMPLES=ON \
    -DNETFORGE_BUILD_TESTS=OFF
```

Build:

```bash
cmake --build build/linux-release --parallel
```

### Windows

Windows builds use Visual Studio and vcpkg for Boost dependencies.

Install Boost through vcpkg:

```powershell
vcpkg install boost-asio:x64-windows
```

Configure a Debug build:

```powershell
cmake -S . -B build/windows-debug `
    -A x64 `
    -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
    -DNETFORGE_BUILD_EXAMPLES=ON `
    -DNETFORGE_BUILD_TESTS=OFF
```

Build:

```powershell
cmake --build build/windows-debug --config Debug --parallel
```

Configure a Release build:

```powershell
cmake -S . -B build/windows-release `
    -A x64 `
    -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
    -DNETFORGE_BUILD_EXAMPLES=ON `
    -DNETFORGE_BUILD_TESTS=OFF
```

Build:

```powershell
cmake --build build/windows-release --config Release --parallel
```

> Replace `C:/vcpkg` with the actual path to your vcpkg installation if it is installed elsewhere.

### Build directories

The recommended build layout is:

```text
build/
├── linux-debug/
├── linux-release/
├── windows-debug/
└── windows-release/
```

Debug and Release builds are kept in separate directories to avoid mixing generated files and build configurations.

## Echo server example

The following example creates a TCP echo server. Every message received from a client is sent back to that client.

```cpp
#include <iostream>
#include <span>

#include <boost/asio.hpp>

#include <netforge/Server.hpp>
#include <netforge/TcpSession.hpp>

class EchoSession : public netforge::TcpSession
{
public:
    using TcpSession::TcpSession;

protected:
    void on_connected() override
    {
        std::cout << "Client connected: " << address() << ':' << port() << std::endl;
    }

    void on_receive(const std::uint8_t* data, std::size_t size) override
    {
        std::cout << "Received " << size << " bytes" << std::endl;
        send(std::span(data, size));
    }

    void on_disconnected() override
    {
        std::cout << "Client disconnected: " << address() << ':' << port() << std::endl;
    }
};

int main()
{
    boost::asio::io_context io;

    netforge::Server<EchoSession> server(io, 5000);

    server.set_error_handler([](const boost::system::error_code& ec) {
        std::cerr << "Netforge server error: " << ec.message() << std::endl;
    });

    server.start();

    std::cout << "Netforge server started on port " << server.port() << std::endl;

    io.run();

    return 0;
}
```

Start the server:

```text
Netforge server started on port 5000
```

Connect any TCP client to port `5000` and send data. The server will return the same data.

## Server

A server is created using the `Server<Session>` template:

```cpp
netforge::Server<EchoSession> server(io, 5000);
```

Start accepting connections:

```cpp
server.start();
```

Stop the server:

```cpp
server.stop();
```

Check whether the server is running:

```cpp
if (server.running())
{
    // ...
}
```

Get the listening port:

```cpp
std::uint16_t port = server.port();
```

Get the number of active sessions:

```cpp
std::size_t count = server.session_count();
```

## Broadcasting

`Server` can send a message to every currently connected session:

```cpp
std::vector<std::uint8_t> message = {
    0x01, 0x02, 0x03, 0x04
};

server.broadcast(message);
```

Each session receives the message through its normal asynchronous send queue.

## Sessions

A custom TCP session is created by inheriting from `netforge::TcpSession`:

```cpp
class MySession : public netforge::TcpSession
{
public:
    using TcpSession::TcpSession;

protected:
    void on_receive(const std::uint8_t* data, std::size_t size) override
    {
        // Handle received data
    }
};
```

### Connection lifecycle

`TcpSession` provides callbacks for connection lifecycle events:

```cpp
void on_connected() override
{
    // Connection established
}

void on_disconnected() override
{
    // Connection closed
}
```

These callbacks can be overridden by user-defined session classes to handle connection events.

### Receiving data

Incoming TCP data is delivered through:

```cpp
void on_receive(const std::uint8_t* data, std::size_t size) override
{
    // Process received data
}
```

The callback receives a pointer to the received data and its size.

The received buffer is only valid for the duration of the callback. If the data needs to be stored, it must be copied.

### Sending data

A session can send data using a `std::vector`:

```cpp
send(std::vector<std::uint8_t>(data, data + size));
```

or a `std::span`:

```cpp
send(std::span<const std::uint8_t>(data, size));
```

Outgoing messages are placed into an internal queue and written asynchronously.

Multiple calls to `send()` are serialized internally, so application code does not need to manually synchronize concurrent writes.

## Session information

A session provides information about its remote peer:

```cpp
address();
port();
remote_endpoint();
```

Example:

```cpp
std::cout << address() << ':' << port() << std::endl;
```

`address()` returns the remote IP address.

`port()` returns the remote TCP port.

`remote_endpoint()` returns the underlying Asio remote endpoint.

## Error handling

Server-level errors can be handled with:

```cpp
server.set_error_handler(
    [](const boost::system::error_code& ec)
    {
        std::cerr << ec.message() << std::endl;
    }
);
```

Session-level errors can be handled by overriding:

```cpp
void on_error(const boost::system::error_code& ec) override
{
    std::cerr << ec.message() << std::endl;
}
```

Errors caused by normal connection termination, such as an EOF when a client closes the connection, may be reported through the session error callback depending on the underlying Asio operation.

## Architecture

Netforge separates server and connection responsibilities:

```text
Server<Session>
      │
      ├── accept
      │
      ├── session management
      │
      └── broadcast
             │
             ▼
         TcpSession
             │
             ├── asynchronous read
             ├── asynchronous write
             ├── write queue
             └── connection callbacks
                    │
                    ▼
              User Session
```

`Server` is responsible for accepting connections, managing active sessions, and broadcasting messages.

`Session` provides common connection state and lifecycle functionality.

`TcpSession` implements TCP-specific asynchronous I/O.

User-defined session classes implement application-specific behavior.

## Building without examples

Examples are enabled by default.

To disable them on Linux:

```bash
cmake -S . -B build/linux-release \
    -DCMAKE_BUILD_TYPE=Release \
    -DNETFORGE_BUILD_EXAMPLES=OFF

cmake --build build/linux-release --parallel
```

On Windows:

```powershell
cmake -S . -B build/windows-release `
    -A x64 `
    -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
    -DNETFORGE_BUILD_EXAMPLES=OFF

cmake --build build/windows-release --config Release --parallel
```

## Building tests

Tests are disabled by default.

Enable them with:

### Linux

```bash
cmake -S . -B build/linux-debug \
    -DCMAKE_BUILD_TYPE=Debug \
    -DNETFORGE_BUILD_TESTS=ON

cmake --build build/linux-debug --parallel
```

### Windows

```powershell
cmake -S . -B build/windows-debug `
    -A x64 `
    -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
    -DNETFORGE_BUILD_TESTS=ON

cmake --build build/windows-debug --config Debug --parallel
```

Tests can then be executed with:

```bash
ctest --test-dir build/linux-debug
```

On Windows:

```powershell
ctest --test-dir build/windows-debug -C Debug
```

## Build output

The Netforge library is built with configuration-specific names.

### Linux

Debug:

```text
build/linux-debug/libNetforge_d.a
```

Release:

```text
build/linux-release/libNetforge.a
```

### Windows

Debug:

```text
build/windows-debug/Debug/Netforge_d.lib
```

Release:

```text
build/windows-release/Release/Netforge.lib
```

The echo example is built as:

### Linux

```text
build/linux-debug/netforge_echo
build/linux-release/netforge_echo
```

### Windows

```text
build/windows-debug/Debug/netforge_echo.exe
build/windows-release/Release/netforge_echo.exe
```

## CMake configuration

Netforge uses the following CMake options:

| Option | Default | Description |
|---|---|---|
| `NETFORGE_BUILD_EXAMPLES` | `ON` | Build example applications |
| `NETFORGE_BUILD_TESTS` | `OFF` | Build tests |

Netforge requires Boost to be available to CMake through `find_package(Boost CONFIG REQUIRED)`.

The project does not download dependencies automatically.

## License

Netforge is licensed under the MIT License.

See [LICENSE](LICENSE) for details.
