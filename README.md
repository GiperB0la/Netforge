# Netforge

**Netforge** is a lightweight asynchronous networking library for modern C++, built on top of **Boost.Asio** and **Boost.Beast**.

It provides a simple, extensible interface for building asynchronous TCP, HTTP and WebSocket clients and servers without having to repeatedly write the same Asio session boilerplate.

> Modern C++ networking without making every project begin with 500 lines of `async_*` callbacks.

## Features

- ⚡ Fully asynchronous networking
- 🧵 Built on Boost.Asio
- 🌐 HTTP client and server support
- 🔌 TCP client and server support
- 🔄 WebSocket client and server support
- 🧩 Session-based architecture
- 🛠️ Easily extensible through inheritance
- 🎯 C++20
- 📦 CMake support
- 🪶 Lightweight API with minimal abstractions

## Requirements

- CMake 3.20+
- C++20 compatible compiler
- Boost

Tested primarily with modern versions of MSVC and Boost.

## Installation

### CMake

Clone the repository:

```bash
git clone https://github.com/GiperB0la/Netforge.git
cd Netforge
```

Configure and build:

```bash
cmake -S . -B build
cmake --build build --config Release
```

Examples are enabled by default.

To disable examples:

```bash
cmake -S . -B build -DNETFORGE_BUILD_EXAMPLES=OFF
```

Tests can be enabled with:

```bash
cmake -S . -B build -DNETFORGE_BUILD_TESTS=ON
```

## CMake Integration

After installing or adding Netforge to your project, link against:

```cmake
find_package(Boost CONFIG REQUIRED)

target_link_libraries(MyApplication
    PRIVATE
        Netforge::Netforge
)
```

Netforge exposes Boost headers through its public interface.

---

# Architecture

Netforge is built around two main concepts:

- **Server / Client** handles connections.
- **Session** handles an individual connection.

The library provides protocol-specific session classes:

```text
                    Session
                       │
          ┌────────────┼────────────┐
          │            │            │
      TcpSession  HttpSession  WebSocketSession
          │            │            │
          │            │            │
   TcpClientSession HttpClient   WebSocketClient
```

Applications can inherit from these classes and override callbacks such as:

```cpp
on_connected()
on_receive(...)
on_request(...)
on_response(...)
on_message(...)
on_disconnected()
```

This keeps networking mechanics inside Netforge while application-specific logic stays in the derived session.

---

# TCP

## TCP Server

A TCP server can be created with `netforge::Server` and a custom `TcpSession`.

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
        std::cout << "Tcp received " << size << " bytes" << std::endl;

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
}
```

The session receives raw bytes through:

```cpp
void on_receive(
    const std::uint8_t* data,
    std::size_t size
) override;
```

Data can be sent using:

```cpp
send("Hello from Netforge!");
```

or:

```cpp
send(std::span(data, size));
```

---

# TCP Client

TCP clients use `netforge::Client` with a custom `TcpSession`.

```cpp
#include <iostream>

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
        std::cout << "Received " << size << " bytes" << std::endl;
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
}
```

---

# HTTP

HTTP functionality is implemented using **Boost.Beast**.

Netforge provides:

- `HttpSession`
- `HttpClientSession`

## HTTP Server

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
}
```

The application only needs to implement request handling:

```cpp
void on_request(const Request& request) override
{
    // Handle HTTP request
}
```

The connection lifecycle and asynchronous I/O are handled by Netforge.

---

# HTTP Client

HTTP clients use `HttpClientSession`.

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

    http_client.set_error_handler([](
        const boost::system::error_code& ec
    ) {
        std::cerr << "HTTP client error: "
                  << ec.message() << std::endl;
    });

    http_client.connect("127.0.0.1", 8080);

    io.run();
}
```

---

# WebSocket

WebSocket support is implemented using Boost.Beast.

Available session types:

```text
WebSocketSession
WebSocketClientSession
```

This allows applications to implement real-time bidirectional communication without manually dealing with the WebSocket handshake and asynchronous read/write operations.

## WebSocket Server

```cpp
#include <iostream>

#include <boost/asio.hpp>

#include <netforge/Server.hpp>
#include <netforge/websocket/WebSocketSession.hpp>

class MyWebSocketSession : public netforge::WebSocketSession
{
public:
    using WebSocketSession::WebSocketSession;

protected:
    void on_connected() override
    {
        std::cout << "WebSocket connected: "
                  << address() << ':' << port() << std::endl;

        send(
            R"({"event":"connected","message":"Hello from Netforge!"})"
        );
    }

    void on_message(std::string_view message) override
    {
        std::cout << "Received: "
                  << message << std::endl;

        send(
            std::string(R"({"event":"message","data":")") +
            std::string(message) +
            R"("})"
        );
    }

    void on_disconnected() override
    {
        std::cout << "WebSocket disconnected: "
                  << address() << ':' << port() << std::endl;
    }
};

int main()
{
    boost::asio::io_context io;

    netforge::Server<MyWebSocketSession> websocket_server(io, 8080);

    websocket_server.set_error_handler([](
        const boost::system::error_code& ec
    ) {
        std::cerr << "WebSocket server error: "
                  << ec.message() << std::endl;
    });

    websocket_server.start();

    std::cout << "Netforge websocket_server started on port "
              << websocket_server.port() << std::endl;

    io.run();
}
```

The server automatically handles the WebSocket handshake before calling:

```cpp
on_connected()
```

Incoming messages are delivered through:

```cpp
void on_message(std::string_view message) override;
```

---

# WebSocket Client

```cpp
#include <iostream>

#include <boost/asio.hpp>

#include <netforge/Client.hpp>
#include <netforge/websocket/WebSocketClientSession.hpp>

class MyWebSocketClientSession
    : public netforge::WebSocketClientSession
{
public:
    using WebSocketClientSession::WebSocketClientSession;

protected:
    void on_connected() override
    {
        std::cout << "WebSocket connected: "
                  << address() << ':' << port() << std::endl;

        send(
            R"({"event":"connected","message":"Hello from Netforge client!"})"
        );
    }

    void on_message(const std::string& message) override
    {
        std::cout << "Received: "
                  << message << std::endl;

        send(
            std::string(R"({"event":"message","data":")") +
            message +
            R"("})"
        );
    }

    void on_disconnected() override
    {
        std::cout << "WebSocket disconnected: "
                  << address() << ':' << port() << std::endl;
    }
};

int main()
{
    boost::asio::io_context io;

    netforge::Client<MyWebSocketClientSession> websocket_client(io);

    websocket_client.set_error_handler([](
        const boost::system::error_code& ec
    ) {
        std::cerr << "WebSocket client error: "
                  << ec.message() << std::endl;
    });

    if (auto* session = websocket_client.session()) {
        session->set_handshake("127.0.0.1", "/ws");
    }

    websocket_client.connect("127.0.0.1", 8080);

    io.run();
}
```

The WebSocket handshake target can be configured using:

```cpp
session->set_handshake("127.0.0.1", "/ws");
```

---

# Error Handling

Both clients and servers support custom error handlers.

```cpp
server.set_error_handler([](
    const boost::system::error_code& ec
) {
    std::cerr << ec.message() << std::endl;
});
```

This allows applications to handle networking errors without exposing the internal asynchronous implementation.

---

# API Overview

## Server

```cpp
netforge::Server<SessionType> server(io, port);
```

Main operations:

```cpp
server.start();
server.port();
server.set_error_handler(...);
```

## Client

```cpp
netforge::Client<SessionType> client(io);
```

Main operations:

```cpp
client.connect(host, port);
client.session();
client.port();
client.set_error_handler(...);
```

## Session

Common session functionality includes:

```cpp
address();
port();
send(...);
```

Connection lifecycle callbacks:

```cpp
on_connected();
on_disconnected();
```

Protocol-specific callbacks are provided by derived session types.

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
│       ├── tcp/
│       │   └── TcpSession.hpp
│       ├── http/
│       │   ├── HttpSession.hpp
│       │   └── HttpClientSession.hpp
│       └── websocket/
│           ├── WebSocketSession.hpp
│           └── WebSocketClientSession.hpp
│
├── src/
│   ├── Session.cpp
│   ├── tcp/
│   │   └── TcpSession.cpp
│   ├── http/
│   │   ├── HttpSession.cpp
│   │   └── HttpClientSession.cpp
│   └── websocket/
│       ├── WebsocketSession.cpp
│       └── WebsocketClientSession.cpp
│
└── examples/
    ├── tcp/
    │   ├── TcpClient.cpp
    │   └── TcpServer.cpp
    ├── http/
    │   ├── HttpClient.cpp
    │   └── HttpServer.cpp
    └── websocket/
        ├── WebsocketClient.cpp
        └── WebsocketServer.cpp
```

---

# Design Goals

Netforge is designed around a few simple principles:

### Minimal abstraction

The library should hide repetitive networking code without hiding the actual networking model.

### Asynchronous by default

All network operations are based on Boost.Asio's asynchronous model.

### Protocol-specific sessions

TCP, HTTP and WebSocket connections have different semantics, so each protocol gets its own session abstraction instead of forcing everything through one giant class.

### Inheritance-based customization

Applications implement their own behavior by deriving from Netforge sessions:

```cpp
class MySession : public netforge::TcpSession
{
    // Application logic
};
```

This keeps the framework small and makes the API predictable.

---

# Dependencies

Netforge currently relies on:

- [Boost.Asio](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html)
- [Boost.Beast](https://www.boost.org/doc/libs/release/libs/beast/)
- C++20 standard library
- CMake

Boost.Asio provides the asynchronous networking layer, while Boost.Beast handles HTTP and WebSocket functionality.

---

# Version

Current version:

```text
0.4.0
```

Netforge is currently under active development, so the API may change between releases. Because apparently software libraries are legally required to occasionally break your code for character development.

---

# License

See the repository license for the current licensing terms.

---

# Roadmap

Planned areas of development include:

- [ ] More comprehensive test suite
- [ ] Improved connection lifecycle management
- [ ] More HTTP features
- [ ] WebSocket improvements
- [ ] TLS / SSL support
- [ ] Better CMake package installation
- [ ] API documentation
- [ ] More examples

---

# Contributing

Contributions, bug reports and improvements are welcome.

If you find a bug or have an idea for improving Netforge, open an issue or submit a pull request.

---

## Why Netforge?

Boost.Asio is powerful, but a basic server still tends to start looking like this:

```cpp
accept();
async_accept();
create_session();
shared_from_this();
async_read();
async_write();
handle_error();
restart_accept();
```

Then three weeks later you've accidentally created your own networking framework.

Netforge is an attempt to stop that process somewhere around the beginning.

```cpp
class EchoSession : public netforge::TcpSession
{
protected:
    void on_receive(
        const std::uint8_t* data,
        std::size_t size
    ) override
    {
        send(std::span(data, size));
    }
};
```

**Simple session logic. Asynchronous underneath.**
