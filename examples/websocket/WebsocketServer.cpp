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
        std::cout << "WebSocket connected: " << address() << ":" << port() << std::endl;
        send(R"({"event":"connected","message":"Hello from Netforge!"})");
    }

    void on_message(std::string_view message) override
    {
        std::cout << "Received: " << message << std::endl;
        send(std::string(R"({"event":"message","data":")") + std::string(message) + R"("})");
    }

    void on_disconnected() override
    {
        std::cout << "WebSocket disconnected: " << address() << ":" << port() << std::endl;
    }
};

int main()
{
    boost::asio::io_context io;

    netforge::Server<MyWebSocketSession> websocket_server(io, 8080);

    websocket_server.set_error_handler([](const boost::system::error_code& ec) {
        std::cerr << "WebSocket server error: " << ec.message() << std::endl;
    });

    websocket_server.start();

    std::cout << "Netforge websocket_server started on port " << websocket_server.port() << std::endl;

    io.run();

	return 0;
}