# Netforge

**Lightweight asynchronous TCP and HTTP networking library for modern C++.**

Netforge is a small object-oriented networking library built on top of [Boost.Asio](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html) and [Boost.Beast](https://www.boost.org/doc/libs/release/libs/beast/).

It provides a simple session-based API for building asynchronous TCP and HTTP servers and clients without manually managing connection lifetimes, asynchronous I/O, write queues, or repetitive networking boilerplate.

The core idea is simple:

> **Keep Boost.Asio's asynchronous model while providing a clean object-oriented networking API.**

---

## Features

- Asynchronous TCP servers
- Asynchronous TCP clients
- Asynchronous HTTP servers
- Asynchronous HTTP clients
- Object-oriented session architecture
- `Server<Session>` API
- `Client<Session>` API
- Automatic session lifetime management
- Asynchronous read/write operations
- Per-session outgoing message queues
- Serialized asynchronous writes
- TCP connection lifecycle callbacks
- HTTP request/response callbacks
- Server-level error handling
- Session-level error handling
- Remote endpoint information
- C++20
- CMake
- Linux and Windows support
- MIT License

---

# Architecture

Netforge uses a shared session-based architecture for both servers and clients.

```text
                    ┌─────────────────────┐
                    │      Netforge       │
                    └──────────┬──────────┘
                               │
                 ┌─────────────┴─────────────┐
                 │                           │
                 ▼                           ▼
          Server<Session>              Client<Session>
                 │                           │
                 ▼                           ▼
             Session                    Session
                 │                           │
          ┌──────┴──────┐            ┌───────┴───────┐
          │             │            │               │
          ▼             ▼            ▼               ▼
    TcpSession    HttpSession   TcpSession    HttpClientSession
          │             │            │               │
          ▼             ▼            ▼               ▼
       TCP I/O       HTTP I/O      TCP I/O         HTTP I/O
```

The transport-independent connection functionality is provided by the session layer, while protocol-specific behavior is implemented by specialized sessions.

### Server

`netforge::Server<Session>` is responsible for:

- accepting incoming TCP connections
- creating session instances
- managing active sessions
- stopping the server
- reporting server-level errors

### Client

`netforge::Client<Session>` is responsible for:

- resolving and connecting to a remote endpoint
- creating the requested session
- managing the client connection
- reporting connection errors

### Session

A session represents a single network connection.

It provides:

- connection lifecycle
- remote endpoint information
- asynchronous I/O
- sending data
- connection callbacks
- error handling

### TcpSession

`TcpSession` provides generic asynchronous TCP communication.

It handles:

- asynchronous reads
- asynchronous writes
- outgoing message queues
- serialized writes
- TCP connection lifecycle

### HttpSession

`HttpSession` provides HTTP server functionality using Boost.Beast.

It handles:

- HTTP request parsing
- asynchronous request processing
- HTTP response sending

### HttpClientSession

`HttpClientSession` provides HTTP client functionality using Boost.Beast.

It handles:

- HTTP request sending
- HTTP response parsing
- asynchronous HTTP communication

Application-specific behavior is implemented by inheriting from the appropriate session class.

---

# Requirements

- C++20 compatible compiler
- CMake 3.20+
- Boost
- Git

Supported compilers include:

- MSVC
- GCC
- Clang

Netforge uses:

- **Boost.Asio** for asynchronous networking
- **Boost.Beast** for HTTP

Boost must be installed separately.

---

# Installation

Clone the repository:

```bash
git clone https://github.com/GiperB0la/Netforge.git
cd Netforge
```

## Linux

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

## Windows

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

A TCP server is created using `Server<TcpSession>`.

Define your own session by inheriting from `netforge::TcpSession`:

```cpp
#include <iostream>

#include <boost/asio.hpp>

#include <netforge/Server.hpp>
#include <netforge/tcp/TcpSession.hpp>

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

The server automatically:

1. accepts incoming connections
2. creates `EchoSession`
3. manages the session lifetime
4. performs asynchronous reads
5. queues outgoing writes
6. serializes writes
7. handles disconnection

The application only needs to implement its protocol logic.

---

# TCP Client

TCP clients use the same `TcpSession` abstraction.

Create a client session by inheriting from `netforge::TcpSession`:

```cpp
#include <iostream>
#include <iomanip>

#include <boost/asio.hpp>

#include <netforge/Client.hpp>
#include <netforge/tcp/TcpSession.hpp>

class MyTcpClientSession : public netforge::TcpSession
{
public:
    using TcpSession::TcpSession;

protected:
    void on_connected() override
    {
        std::cout << "TCP connected: "
                  << address() << ':' << port() << std::endl;

        send("Hello from Netforge client!");
    }

    void on_receive(const std::uint8_t* data, std::size_t size) override
    {
        std::cout << "Tcp received " << size << " bytes: ";

        for (std::size_t i = 0; i < size; ++i) {
            std::cout << std::hex
                      << std::setw(2)
                      << std::setfill('0')
                      << static_cast<int>(data[i])
                      << ' ';
        }

        std::cout << std::dec << std::endl;
    }

    void on_disconnected() override
    {
        std::cout << "TCP disconnected" << std::endl;
    }
};

int main()
{
    boost::asio::io_context io;

    netforge::Client<MyTcpClientSession> tcp_client(io);

    tcp_client.set_error_handler([](const boost::system::error_code& ec) {
        std::cerr << "Tcp client error: "
                  << ec.message() << std::endl;
    });

    tcp_client.connect("127.0.0.1", 5000);

    io.run();

    return 0;
}
```

Connect to a server with:

```cpp
tcp_client.connect("127.0.0.1", 5000);
```

After the connection is established, `on_connected()` is called.

Incoming data is delivered through:

```cpp
void on_receive(
    const std::uint8_t* data,
    std::size_t size
) override;
```

---

# HTTP Server

Netforge provides `HttpSession` for asynchronous HTTP server development using Boost.Beast.

```cpp
#include <iostream>

#include <boost/asio.hpp>

#include <netforge/Server.hpp>
#include <netforge/http/HttpSession.hpp>

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

Run the server and open:

```text
http://localhost:8080/
```

The server responds with:

```text
Hello from Netforge!
```

HTTP requests are delivered through:

```cpp
void on_request(
    const boost::beast::http::request<
        boost::beast::http::string_body
    >& request
) override;
```

Responses are sent asynchronously using:

```cpp
send(std::move(response));
```

---

# HTTP Client

HTTP clients use `netforge::HttpClientSession`.

```cpp
#include <iostream>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <netforge/Client.hpp>
#include <netforge/http/HttpClientSession.hpp>

class MyHttpClientSession : public netforge::HttpClientSession
{
public:
    using HttpClientSession::HttpClientSession;

protected:
    void on_connected() override
    {
        std::cout << "HTTP client connected: "
                  << address() << ':' << port() << std::endl;

        boost::beast::http::request<
            boost::beast::http::string_body
        > request{
            boost::beast::http::verb::get,
            "/",
            11
        };

        request.set(
            boost::beast::http::field::host,
            address()
        );

        request.set(
            boost::beast::http::field::user_agent,
            "Netforge"
        );

        request.keep_alive(true);

        send(std::move(request));
    }

    void on_response(
        const boost::beast::http::response<
            boost::beast::http::string_body
        >& response
    ) override
    {
        std::cout << "HTTP response:" << std::endl;
        std::cout << "Status: "
                  << response.result_int() << std::endl;

        std::cout << "Body: "
                  << response.body() << std::endl;
    }

    void on_disconnected() override
    {
        std::cout << "HTTP client disconnected: "
                  << address() << ':' << port() << std::endl;
    }
};

int main()
{
    boost::asio::io_context io;

    netforge::Client<MyHttpClientSession> http_client(io);

    http_client.set_error_handler([](const boost::system::error_code& ec) {
        std::cerr << "HTTP client error: "
                  << ec.message() << std::endl;
    });

    http_client.connect("127.0.0.1", 8080);

    io.run();

    return 0;
}
```

Connect using:

```cpp
http_client.connect("127.0.0.1", 8080);
```

After the connection is established, `on_connected()` is called.

HTTP responses are delivered through:

```cpp
void on_response(
    const boost::beast::http::response<
        boost::beast::http::string_body
    >& response
) override;
```

---

# Session Lifecycle

Sessions expose connection lifecycle callbacks.

## Connected

Called after the connection has been established:

```cpp
void on_connected() override
{
    std::cout << "Connected\n";
}
```

## Disconnected

Called when the connection is closed:

```cpp
void on_disconnected() override
{
    std::cout << "Disconnected\n";
}
```

## Error

Session errors can be handled by overriding:

```cpp
void on_error(
    const boost::system::error_code& ec
) override
{
    std::cerr << "Session error: "
              << ec.message() << '\n';
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

# Sending TCP Data

`TcpSession` provides asynchronous sending.

Send data using `std::span`:

```cpp
send(std::span<const std::uint8_t>(data, size));
```

Send a vector:

```cpp
send(std::vector<std::uint8_t>{
    0x01,
    0x02,
    0x03
});
```

Send a string:

```cpp
send("Hello from Netforge!");
```

Outgoing messages are placed into an internal queue and written asynchronously.

Multiple `send()` calls are serialized internally.

Application code does not need to manually synchronize concurrent writes.

---

# Receiving TCP Data

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

The buffer is only valid for the duration of the callback.

If the data needs to be stored after the callback returns, it must be copied.

For example:

```cpp
std::vector<std::uint8_t> packet(data, data + size);
```

---

# Error Handling

Server-level errors can be handled using:

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

Client-level errors use the same interface:

```cpp
client.set_error_handler(
    [](const boost::system::error_code& ec)
    {
        std::cerr << "Client error: "
                  << ec.message()
                  << '\n';
    }
);
```

Session-specific errors can be handled by overriding `on_error()`.

Normal connection termination may produce errors such as `boost::asio::error::eof`, depending on the underlying asynchronous operation.

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
if (server.running()) {
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

# Client API

Create a client:

```cpp
netforge::Client<MySession> client(io);
```

Connect to a remote endpoint:

```cpp
client.connect("127.0.0.1", 5000);
```

Get the connection port:

```cpp
std::uint16_t port = client.port();
```

Set an error handler:

```cpp
client.set_error_handler(
    [](const boost::system::error_code& ec) {
        std::cerr << ec.message() << '\n';
    }
);
```

The connection lifecycle is handled by the configured session.

---

# CMake

Netforge provides CMake options for controlling optional components.

| Option | Default | Description |
|---|---:|---|
| `NETFORGE_BUILD_EXAMPLES` | `ON` | Build example applications |
| `NETFORGE_BUILD_TESTS` | `OFF` | Build tests |

Example:

```bash
cmake -S . -B build \
    -DNETFORGE_BUILD_EXAMPLES=ON \
    -DNETFORGE_BUILD_TESTS=ON
```

Build:

```bash
cmake --build build --parallel
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
│       ├── Client.hpp
│       ├── Server.hpp
│       ├── Session.hpp
│       │
│       ├── tcp/
│       │   └── TcpSession.hpp
│       │
│       └── http/
│           ├── HttpSession.hpp
│           └── HttpClientSession.hpp
│
├── src/
│   ├── Client.cpp
│   ├── Server.cpp
│   ├── Session.cpp
│   ├── tcp/
│   │   └── TcpSession.cpp
│   └── http/
│       ├── HttpSession.cpp
│       └── HttpClientSession.cpp
│
├── examples/
│   ├── tcp_server/
│   │   └── main.cpp
│   ├── tcp_client/
│   │   └── main.cpp
│   ├── http_server/
│   │   └── main.cpp
│   └── http_client/
│       └── main.cpp
│
├── tests/
└── README.md
```

---

# Dependencies

Netforge intentionally keeps its dependency stack small.

### Boost.Asio

Used for:

- asynchronous networking
- TCP sockets
- connection handling
- asynchronous I/O

### Boost.Beast

Used for:

- HTTP request parsing
- HTTP response parsing
- HTTP client/server communication

Boost is an external dependency and must be installed separately.

---

# Design Goals

## Simple API

Common networking tasks should require as little boilerplate as possible.

A basic server can be created with:

```cpp
netforge::Server<MySession> server(io, 5000);
server.start();

io.run();
```

A client can be connected with:

```cpp
netforge::Client<MySession> client(io);
client.connect("127.0.0.1", 5000);

io.run();
```

## Asynchronous by Default

Netforge is built around Boost.Asio's asynchronous execution model.

The library does not introduce a separate event loop. Applications use the normal:

```cpp
boost::asio::io_context io;
io.run();
```

model.

## Session-Based Architecture

Each network connection is represented by a session object.

This keeps protocol-specific logic isolated from connection management.

For example:

```text
Server
  │
  ├── TcpSession
  ├── TcpSession
  └── TcpSession
```

and:

```text
Client
  │
  └── TcpSession
```

The same session abstraction can therefore be used on both sides of a TCP connection.

## Safe Asynchronous Writes

Outgoing messages are queued and serialized internally.

Application code can call:

```cpp
send(data);
send(other_data);
send(message);
```

without manually implementing a write queue for every connection.

## Small Scope

Netforge is not intended to replace Boost.Asio.

It provides a higher-level server/client/session layer on top of it while keeping direct access to Boost types where appropriate.

---

# Examples

The repository contains complete examples for the supported networking models:

```text
examples/
├── tcp_server/
├── tcp_client/
├── http_server/
└── http_client/
```

### TCP

```text
TCP Server <----> TCP Client
```

### HTTP

```text
HTTP Server <----> HTTP Client
```

All examples use the same `io_context`-based asynchronous model.

---

# License

Netforge is released under the **MIT License**.

See [LICENSE](LICENSE) for the full license text.

---

# Repository

GitHub:

https://github.com/GiperB0la/Netforge
