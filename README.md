# Netforge

Lightweight asynchronous TCP networking library for modern C++.

Netforge provides a simple object-oriented API for building asynchronous TCP servers on top of Asio. It handles accepting connections, session lifetime, asynchronous reads and writes, write queues, and safe access to session state.

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
- Git
- Asio

The project uses CMake `FetchContent` to obtain Asio automatically when it is not already available on the system.

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
│       └── main.cpp
└── README.md
```

## Quick start

Clone the repository:

```bash
git clone https://github.com/GiperB0la/Netforge.git
cd Netforge
```

Create a build directory:

```bash
mkdir build
cd build
```

Configure the project:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

Build:

```bash
cmake --build . -j$(nproc)
```

On Windows with Visual Studio:

```powershell
cmake .. 
cmake --build . --config Release
```

The example application will be built as:

```text
netforge_echo
```

On Windows:

```text
Release/netforge_echo.exe
```

## Debug build

Linux:

```bash
mkdir build_linux_debug
cd build_linux_debug

cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j$(nproc)
```

The Debug library is named:

```text
libNetforge_d.a
```

## Release build

Linux:

```bash
mkdir build_linux_release
cd build_linux_release

cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

The Release library is named:

```text
libNetforge.a
```

On Windows the corresponding names are:

```text
Netforge_d.lib
Netforge.lib
```

## Echo server example

The following example creates a TCP echo server. Every message received from a client is sent back to that client.

```cpp
#include <iostream>

#include <boost/asio.hpp>

#include <netforge/Server.hpp>
#include <netforge/TcpSession.hpp>

class EchoSession : public netforge::TcpSession
{
public:
    using TcpSession::TcpSession;

protected:
    void on_receive(const std::uint8_t* data, std::size_t size) override
    {
        send(std::vector<std::uint8_t>(data, data + size));
    }

    void on_start() override
    {
        std::cout << "Client connected: "
                  << address() << ':' << port()
                  << std::endl;
    }

    void on_stop() override
    {
        std::cout << "Client disconnected"
                  << std::endl;
    }
};

int main()
{
    boost::asio::io_context io;

    netforge::Server<EchoSession> server(io, 5000);

    server.set_error_handler([](const boost::system::error_code& ec) {
        std::cerr << "Netforge server error: "
                  << ec.message()
                  << std::endl;
    });

    server.start();

    std::cout << "Netforge server started on port "
              << server.port()
              << std::endl;

    io.run();
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
    void on_receive(
        const std::uint8_t* data,
        std::size_t size
    ) override
    {
        // Handle received data
    }
};
```

### Connection lifecycle

`TcpSession` provides lifecycle callbacks:

```cpp
void on_start() override
{
    // Connection started
}

void on_stop() override
{
    // Connection stopped
}
```

### Receiving data

Incoming TCP data is delivered through:

```cpp
void on_receive(
    const std::uint8_t* data,
    std::size_t size
) override
{
    // Process data
}
```

### Sending data

A session can send data using:

```cpp
send(std::vector<std::uint8_t>(data, data + size));
```

or:

```cpp
send(std::span<const std::uint8_t>(data, size));
```

Outgoing messages are placed into an internal queue and written asynchronously.

## Session information

A session provides information about its remote peer:

```cpp
address()
port()
remote_endpoint()
```

Example:

```cpp
std::cout << address()
          << ':'
          << port()
          << std::endl;
```

## Error handling

Server errors can be handled with:

```cpp
server.set_error_handler(
    [](const boost::system::error_code& ec)
    {
        std::cerr << ec.message() << std::endl;
    }
);
```

Session errors can be handled by overriding:

```cpp
void on_error(const boost::system::error_code& ec) override
{
    std::cerr << ec.message() << std::endl;
}
```

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

`Server` is responsible for accepting connections and managing active sessions.

`Session` provides the common connection lifecycle and networking state.

`TcpSession` implements TCP-specific asynchronous I/O.

User-defined session classes implement application-specific behavior.

## Building without examples

Examples are enabled by default.

To disable them:

```bash
cmake .. -DNETFORGE_BUILD_EXAMPLES=OFF
cmake --build .
```

## Building tests

Tests can be enabled with:

```bash
cmake .. -DNETFORGE_BUILD_TESTS=ON
cmake --build .
```

## License

See the `LICENSE` file in the repository.
