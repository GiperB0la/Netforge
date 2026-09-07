# Netforge

Lightweight asynchronous TCP and HTTP networking library for modern C++.

Netforge provides a simple object-oriented API for building asynchronous network servers on top of Boost.Asio and Boost.Beast. It handles accepting connections, session lifetime, asynchronous reads and writes, write queues, connection lifecycle callbacks, and safe access to session state.

## Features

- Asynchronous TCP server
- Asynchronous HTTP server
- Object-oriented session architecture
- `Server<Session>` template
- `TcpSession` with asynchronous read/write
- `HttpSession` with asynchronous HTTP request/response handling
- Thread-safe writes through Asio strands
- Automatic session lifetime management
- Per-session outgoing message queue
- Broadcast messages to all connected TCP sessions
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

Netforge uses:

- Boost.Asio for asynchronous networking
- Boost.Beast for HTTP

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
│       ├── HttpSession.hpp
│       ├── Server.hpp
│       ├── Session.hpp
│       └── TcpSession.hpp
├── src/
│   ├── HttpSession.cpp
│   ├── Session.cpp
│   └── TcpSession.cpp
├── examples/
│   ├── tcp_server/
│   │   └── main.cpp
│   └── http_server/
│       └── main.cpp
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

### Debug

Configure:

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

### Release

Configure:

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
vcpkg install boost:x64-windows
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

## TCP server

A custom TCP session is created by inheriting from `netforge::TcpSession`:

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
        std::cout << "Tcp client connected: "
                  << address() << ':' << port() << std::endl;
    }

    void on_receive(const std::uint8_t* data, std::size_t size) override
    {
        std::cout << "Tcp received "
                  << size << " bytes" << std::endl;

        send(std::span(data, size));
    }

    void on_disconnected() override
    {
        std::cout << "Tcp client disconnected: "
                  << address() << ':' << port() << std::endl;
    }
};

int main()
{
    boost::asio::io_context io;

    netforge::Server<EchoSession> tcp_server(io, 5000);

    tcp_server.set_error_handler([](const boost::system::error_code& ec) {
        std::cerr << "Tcp server error: "
                  << ec.message() << std::endl;
    });

    tcp_server.start();

    std::cout << "Netforge tcp_server started on port "
              << tcp_server.port() << std::endl;

    io.run();

    return 0;
}
```

The example is available in:

```text
examples/tcp_server/main.cpp
```

Start the server:

```text
Netforge tcp_server started on port 5000
```

Connect any TCP client to port `5000` and send data. The server will return the same data.

## HTTP server

Netforge provides `HttpSession` for asynchronous HTTP request/response handling using Boost.Beast.

A custom HTTP session is created by inheriting from `netforge::HttpSession`:

```cpp
#include <iostream>

#include <boost/asio.hpp>

#include <netforge/Server.hpp>
#include <netforge/HttpSession.hpp>

class MyHttpSession : public netforge::HttpSession
{
public:
    using HttpSession::HttpSession;

protected:
    void on_connected() override
    {
        std::cout << "Http client connected: "
                  << address() << ':' << port() << std::endl;
    }

    void on_request(
        const boost::beast::http::request<boost::beast::http::string_body>& request
    ) override
    {
        boost::beast::http::response<boost::beast::http::string_body> response{
            boost::beast::http::status::ok,
            request.version()
        };

        response.keep_alive(request.keep_alive());
        response.set(
            boost::beast::http::field::content_type,
            "text/plain"
        );

        response.body() = "Hello from Netforge!";
        response.prepare_payload();

        send(std::move(response));
    }

    void on_disconnected() override
    {
        std::cout << "Http client disconnected: "
                  << address() << ':' << port() << std::endl;
    }
};

int main()
{
    boost::asio::io_context io;

    netforge::Server<MyHttpSession> http_server(io, 8080);

    http_server.set_error_handler([](const boost::system::error_code& ec) {
        std::cerr << "Http server error: "
                  << ec.message() << std::endl;
    });

    http_server.start();

    std::cout << "Netforge http_server started on port "
              << http_server.port() << std::endl;

    io.run();

    return 0;
}
```

The example is available in:

```text
examples/http_server/main.cpp
```

Start the server:

```text
Netforge http_server started on port 8080
```

Then open:

```text
http://localhost:8080/
```

The server responds with:

```text
Hello from Netforge!
```

## Server

A server is created using the `Server<Session>` template:

```cpp
netforge::Server<MySession> server(io, 5000);
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

`Server` can send a message to every currently connected TCP session:

```cpp
std::vector<std::uint8_t> message = {
    0x01, 0x02, 0x03, 0x04
};

server.broadcast(message);
```

Each session receives the message through its normal asynchronous send queue.

## Sessions

### TCP sessions

TCP sessions inherit from `netforge::TcpSession`:

```cpp
class MySession : public netforge::TcpSession
{
public:
    using TcpSession::TcpSession;

protected:
    void on_receive(
        const std::uint8_t* data,
        std::size_t size
    ) override
    {
        // Handle received data
    }
};
```

### HTTP sessions

HTTP sessions inherit from `netforge::HttpSession`:

```cpp
class MyHttpSession : public netforge::HttpSession
{
public:
    using HttpSession::HttpSession;

protected:
    void on_request(
        const boost::beast::http::request<
            boost::beast::http::string_body
        >& request
    ) override
    {
        // Handle request
    }
};
```

## Connection lifecycle

Sessions provide callbacks for connection lifecycle events:

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

## Receiving TCP data

Incoming TCP data is delivered through:

```cpp
void on_receive(
    const std::uint8_t* data,
    std::size_t size
) override
{
    // Process received data
}
```

The callback receives a pointer to the received data and its size.

The received buffer is only valid for the duration of the callback. If the data needs to be stored, it must be copied.

## Sending TCP data

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

## Sending HTTP responses

`HttpSession` provides an asynchronous `send()` overload for HTTP responses:

```cpp
boost::beast::http::response<boost::beast::http::string_body> response{
    boost::beast::http::status::ok,
    request.version()
};

response.keep_alive(request.keep_alive());

response.set(
    boost::beast::http::field::content_type,
    "text/plain"
);

response.body() = "Hello from Netforge!";
response.prepare_payload();

send(std::move(response));
```

HTTP request handling is performed through:

```cpp
void on_request(
    const boost::beast::http::request<
        boost::beast::http::string_body
    >& request
) override
{
    // Handle request
}
```

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

Netforge separates server, transport, and application responsibilities:

```text
                    Server<Session>
                          │
                          ├── accept
                          │
                          ├── session management
                          │
                          └── broadcast
                               │
                ┌──────────────┴──────────────┐
                │                             │
                ▼                             ▼
           TcpSession                    HttpSession
                │                             │
                ├── async read                ├── HTTP read
                ├── async write               ├── HTTP parse
                ├── write queue               ├── HTTP response
                └── callbacks                 └── callbacks
                │                             │
                ▼                             ▼
          User TCP Session              User HTTP Session
```

`Server` is responsible for accepting connections, managing active sessions, and broadcasting messages.

`Session` provides common connection state and lifecycle functionality.

`TcpSession` implements TCP-specific asynchronous I/O.

`HttpSession` implements HTTP-specific asynchronous I/O using Boost.Beast.

User-defined session classes implement application-specific behavior.

## Building without examples

Examples are enabled by default.

To disable them on Linux:

```bash
cmake -S . -B build/linux-release \
    -DCMAKE_BUILD_TYPE=Release \
    -DNETFORGE_BUILD_EXAMPLES=OFF \
    -DNETFORGE_BUILD_TESTS=OFF

cmake --build build/linux-release --parallel
```

On Windows:

```powershell
cmake -S . -B build/windows-release `
    -A x64 `
    -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
    -DNETFORGE_BUILD_EXAMPLES=OFF `
    -DNETFORGE_BUILD_TESTS=OFF

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

Examples are built as:

### Linux

```text
build/linux-debug/netforge_tcp_server
build/linux-debug/netforge_http_server

build/linux-release/netforge_tcp_server
build/linux-release/netforge_http_server
```

### Windows

```text
build/windows-debug/Debug/netforge_tcp_server.exe
build/windows-debug/Debug/netforge_http_server.exe

build/windows-release/Release/netforge_tcp_server.exe
build/windows-release/Release/netforge_http_server.exe
```

## CMake configuration

Netforge uses the following CMake options:

| Option | Default | Description |
|---|---|---|
| `NETFORGE_BUILD_EXAMPLES` | `ON` | Build example applications |
| `NETFORGE_BUILD_TESTS` | `OFF` | Build tests |

Netforge requires Boost to be available to CMake through:

```cmake
find_package(Boost CONFIG REQUIRED)
```

The project does not download dependencies automatically.

## License

Netforge is licensed under the MIT License.

See [LICENSE](LICENSE) for details.
