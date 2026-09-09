#include <iostream>

#include <boost/asio.hpp>

#include <netforge/Client.hpp>
#include <netforge/websocket/WebSocketClientSession.hpp>

class MyWebSocketClientSession : public netforge::WebSocketClientSession
{
public:
    using WebSocketClientSession::WebSocketClientSession;

protected:
    void on_connected() override
    {
        std::cout << "WebSocket connected: " << address() << ":" << port() << std::endl;
        send(R"({"event":"connected","message":"Hello from Netforge client!"})");
    }

    void on_message(const std::string& message) override
    {
        std::cout << "Received: " << message << std::endl;

        send(
            std::string(R"({"event":"message","data":")") +
            message +
            R"("})"
        );
    }

    void on_disconnected() override
    {
        std::cout << "WebSocket disconnected: " << address() << ":" << port() << std::endl;
    }
};

int main()
{
    boost::asio::io_context io;

    netforge::Client<MyWebSocketClientSession> websocket_client(io);

    websocket_client.set_error_handler([](const boost::system::error_code& ec) {
        std::cerr << "WebSocket client error: " << ec.message() << std::endl;
    });

    if (auto* session = websocket_client.session()) {
        session->set_handshake("127.0.0.1", "/ws");
    }

    websocket_client.connect("127.0.0.1", 8080);

    io.run();

    return 0;
}