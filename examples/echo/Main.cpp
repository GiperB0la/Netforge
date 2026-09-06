#include <iostream>

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