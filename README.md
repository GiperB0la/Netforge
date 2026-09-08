# Netforge

**Lightweight asynchronous TCP and HTTP networking library for modern C++.**

Netforge is a small object-oriented networking library built on top of [Boost.Asio](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html) and [Boost.Beast](https://www.boost.org/doc/libs/release/libs/beast/).

It provides a simple session-based API for building asynchronous TCP and HTTP servers without having to manage the low-level connection lifecycle, asynchronous write queues, or synchronization yourself.

```cpp
netforge::Server<MySession> server(io, 8080);
server.start();

io.run();
```

The goal of Netforge is simple:

> **Keep Boost.Asio's asynchronous model while removing repetitive server boilerplate.**

---

## Features

- Asynchronous TCP servers
- Asynchronous HTTP servers
- `Server<Session>` architecture
- Object-oriented session model
- Automatic session lifetime management
- Asynchronous read/write operations
- Per-session outgoing message queues
- Thread-safe writes using Asio strands
- TCP connection lifecycle callbacks
- Session-level error handling
- Server-level error handling
- TCP broadcasting to connected sessions
- Remote endpoint information
- C++20
- CMake
- Linux and Windows support
- MIT License

---

## Why Netforge?

Boost.Asio is powerful, but building a complete server around it usually means writing a fair amount of infrastructure:

- accept loops
- session ownership
- connection lifetime management
- asynchronous reads
- asynchronous writes
- write serialization
- outgoing queues
- connection callbacks
- error handling

Netforge handles this infrastructure so application code can focus on the actual protocol or application logic.

Instead of building the same networking boilerplate for every project:

```text
accept
  ↓
create socket
  ↓
create session
  ↓
manage lifetime
  ↓
async_read
  ↓
process data
  ↓
queue async_write
  ↓
handle disconnect
  ↓
repeat
```

you define a session and implement your application logic.

---

# Quick Start

## Requirements

- C++20 compatible compiler
- CMake 3.20+
- Boost
- Git

Supported compilers:

- MSVC
- GCC
- Clang

Netforge uses:

- **Boost.Asio** for asynchronous networking
- **Boost.Beast** for HTTP

Boost must be installed separately. Netforge does not download dependencies automatically.

---

## Installation

Clone the repository:

```bash
git clone https://github.com/GiperB0la/Netforge.git
cd Netforge
```

### Linux

On Debian/Ubuntu:

```bash
sudo apt update
sudo apt install build-essential cmake libboost-all-dev
```

Configure and build:

```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DNETFORGE_BUILD_EXAMPLES=ON

cmake --build build --parallel
```

### Windows

Install Boost using vcpkg:

```powershell
vcpkg install boost:x64-windows
```

Configure:

```powershell
cmake -S . -B build `
    -A x64 `
    -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
    -DNETFORGE_BUILD_EXAMPLES=ON
```

Build:

```powershell
cmake --build build --config Release --parallel
```

Replace `C:/vcpkg` with your actual vcpkg installation path.

---

# TCP Server

The basic Netforge building block is a session.

Create a TCP server by inheriting from `netforge::TcpSession`:

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
        std::cout << "Client connected: "
                  << address() << ':' << port() << '\n';
    }

    void on_receive(const std::uint8_t* data, std::size_t size) override
    {
        send(std::span(data, size));
    }

    void on_disconnected() override
    {
        std::cout << "Client disconnected\n";
    }
};

int main()
{
    boost::asio::io_context io;

    netforge::Server<EchoSession> server(io, 5000);

    server.start();

    std::cout << "Server listening on port "
              << server.port() << '\n';

    io.run();
}
```

That's it.

The server accepts connections, creates `EchoSession` instances, manages their lifetime, performs asynchronous I/O, and serializes outgoing writes.

The complete example is available at:

```text
examples/tcp_server/main.cpp
```

You can test it with any TCP client:

```text
localhost:5000
```

The example implements a simple echo server: every received message is sent back to the client.

---

# HTTP Server

Netforge also provides `HttpSession` for asynchronous HTTP handling using Boost.Beast.

```cpp
#include <iostream>

#include <boost/asio.hpp>

#include <netforge/Server.hpp>
#include <netforge/HttpSession.hpp>

class HttpSession : public netforge::HttpSession
{
public:
    using netforge::HttpSession::HttpSession;

protected:
    void on_request(
        const boost::beast::http::request<
            boost::beast::http::string_body
        >& request
    ) override
    {
        boost::beast::http::response<
            boost::beast::http::string_body
        > response{
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
};

int main()
{
    boost::asio::io_context io;

    netforge::Server<HttpSession> server(io, 8080);

    server.start();

    std::cout << "HTTP server listening on port "
              << server.port() << '\n';

    io.run();
}
```

Run the server and open:

```text
http://localhost:8080/
```

The server responds with:

```text
Hello from Netforge!
```

Complete example:

```text
examples/http_server/main.cpp
```

---

# Architecture

Netforge uses a simple layered architecture:

```text
                    Server<Session>
                          │
                    accepts TCP
                          │
                          ▼
                       Session
                          │
             ┌────────────┴────────────┐
             │                         │
             ▼                         ▼
        TcpSession                HttpSession
             │                         │
       async TCP I/O             Boost.Beast
             │                         │
             ▼                         ▼
      User TCP Session          User HTTP Session
```

### `Server<Session>`

Responsible for:

- accepting connections
- creating sessions
- tracking active sessions
- stopping the server
- broadcasting messages

### `Session`

Provides common connection functionality:

- connection state
- remote endpoint information
- lifecycle callbacks
- error handling

### `TcpSession`

Provides:

- asynchronous TCP reads
- asynchronous TCP writes
- outgoing message queue
- serialized writes
- TCP-specific callbacks

### `HttpSession`

Provides:

- HTTP request parsing
- asynchronous HTTP handling
- HTTP response sending
- Boost.Beast integration

Application-specific behavior is implemented in user-defined session classes.

---

# Server API

Create a server:

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

---

# Broadcasting

TCP servers can broadcast data to all currently connected sessions:

```cpp
std::vector<std::uint8_t> message = {
    0x01, 0x02, 0x03, 0x04
};

server.broadcast(message);
```

Each session receives the message through its normal asynchronous send queue.

Application code does not need to manually synchronize concurrent writes.

---

# TCP Sessions

Create a custom TCP session by inheriting from `TcpSession`:

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

## Receiving data

Incoming data is delivered through:

```cpp
void on_receive(
    const std::uint8_t* data,
    std::size_t size
) override
{
    // Process data
}
```

The buffer is only valid for the duration of the callback.

If the data needs to be stored after the callback returns, it must be copied.

---

## Sending data

Send a `std::span`:

```cpp
send(std::span<const std::uint8_t>(data, size));
```

or a vector:

```cpp
send(std::vector<std::uint8_t>{
    0x01, 0x02, 0x03
});
```

Outgoing messages are placed into an internal queue and written asynchronously.

Multiple `send()` calls are serialized internally.

---

# HTTP Sessions

Create a custom HTTP session:

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
        // Handle HTTP request
    }
};
```

HTTP requests are delivered through `on_request()`.

Responses can be sent asynchronously:

```cpp
boost::beast::http::response<
    boost::beast::http::string_body
> response{
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

---

# Connection Lifecycle

Sessions provide connection lifecycle callbacks.

When a connection is established:

```cpp
void on_connected() override
{
    std::cout << "Connected\n";
}
```

When a connection is closed:

```cpp
void on_disconnected() override
{
    std::cout << "Disconnected\n";
}
```

This makes it easy to implement application-specific connection handling.

For example:

```cpp
void on_connected() override
{
    std::cout << "Client connected: "
              << address() << ':' << port() << '\n';
}

void on_disconnected() override
{
    std::cout << "Client disconnected\n";
}
```

---

# Session Information

A session provides information about the remote peer:

```cpp
address();
port();
remote_endpoint();
```

Example:

```cpp
std::cout << address()
          << ':'
          << port()
          << '\n';
```

`address()` returns the remote IP address.

`port()` returns the remote TCP port.

`remote_endpoint()` returns the underlying Boost.Asio remote endpoint.

---

# Error Handling

Server-level errors can be handled with:

```cpp
server.set_error_handler(
    [](const boost::system::error_code& ec)
    {
        std::cerr << "Server error: "
                  << ec.message()
                  << '\n';
    }
);
```

Session-level errors can be handled by overriding:

```cpp
void on_error(
    const boost::system::error_code& ec
) override
{
    std::cerr << "Session error: "
              << ec.message()
              << '\n';
}
```

Normal connection termination may result in errors such as `eof` depending on the underlying Asio operation.

---

# CMake Options

Netforge provides the following CMake options:

| Option | Default | Description |
|---|---:|---|
| `NETFORGE_BUILD_EXAMPLES` | `ON` | Build example applications |
| `NETFORGE_BUILD_TESTS` | `OFF` | Build tests |

For example:

```bash
cmake -S . -B build \
    -DNETFORGE_BUILD_EXAMPLES=OFF \
    -DNETFORGE_BUILD_TESTS=ON
```

---

# Tests

Tests are disabled by default.

Enable them with:

```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DNETFORGE_BUILD_TESTS=ON

cmake --build build --parallel
```

Run:

```bash
ctest --test-dir build
```

On Windows:

```powershell
ctest --test-dir build -C Debug
```

---

# Project Structure

```text
Netforge/
├── CMakeLists.txt
├── include/
│   └── netforge/
│       ├── HttpSession.hpp
│       ├── Server.hpp
│       ├── Session.hpp
│       └── TcpSession.hpp
│
├── src/
│   ├── HttpSession.cpp
│   ├── Session.cpp
│   └── TcpSession.cpp
│
├── examples/
│   ├── tcp_server/
│   │   └── main.cpp
│   └── http_server/
│       └── main.cpp
│
├── tests/
│
└── README.md
```

---

# Dependencies

Netforge intentionally keeps its dependency stack small.

### Boost.Asio

Used for asynchronous networking, sockets, connection handling, and I/O.

### Boost.Beast

Used for HTTP protocol handling.

Boost is an external dependency and must be installed separately.

---

# Design Goals

Netforge is designed around a few principles:

### Simple API

Common networking tasks should require as little boilerplate as possible.

### Asynchronous by default

Network operations are asynchronous and integrated with Boost.Asio's `io_context`.

### Explicit ownership

Sessions are managed automatically by the server while still allowing application code to work with normal C++ classes.

### Safe concurrent writes

Outgoing data is serialized internally using per-session queues and Asio strands.

### Small scope

Netforge is not intended to replace Boost.Asio.

It provides a higher-level server/session layer on top of it.

---

# Roadmap

Planned improvements include:

- [ ] More comprehensive test coverage
- [ ] More TCP server examples
- [ ] HTTP routing
- [ ] WebSocket sessions
- [ ] TLS/SSL support
- [ ] Package manager support
- [ ] API documentation
- [ ] Performance benchmarks
- [ ] More complete examples and documentation

The API may evolve as the project develops.

---

# Contributing

Contributions, bug reports, feature requests, and improvements are welcome.

Before submitting a pull request:

1. Build the project.
2. Run the existing tests.
3. Make sure examples still compile.
4. Keep changes focused.
5. Follow the existing C++ style.

For bugs and feature requests, open an issue on GitHub.

---

# License

Netforge is released under the **MIT License**.

See [LICENSE](LICENSE) for the full license text.

---

## Repository

GitHub:

https://github.com/GiperB0la/Netforge
