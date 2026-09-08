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
        std::cout << "Tcp client connected: " << address() << ':' << port() << std::endl;
    }

    void on_receive(const std::uint8_t* data, std::size_t size) override
    {
        std::cout << "Tcp received " << size << " bytes" << std::endl;
        send(std::span(data, size));
    }

    void on_disconnected() override
    {
        std::cout << "Tcp client disconnected: " << address() << ':' << port() << std::endl;
    }
};

int main()
{
   boost::asio::io_context io;

   netforge::Server<EchoSession> tcp_server(io, 5000);

   tcp_server.set_error_handler([](const boost::system::error_code& ec) {
       std::cerr << "Tcp server error: " << ec.message() << std::endl;
   });

   tcp_server.start();

   std::cout << "Netforge tcp_server started on port " << tcp_server.port() << std::endl;

   io.run();

   return 0;
}