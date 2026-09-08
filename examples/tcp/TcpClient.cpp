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
        std::cout << "TCP connected: " << address() << ':' << port() << std::endl;
        send("Hello from Netforge client!");
    }

    void on_receive(const std::uint8_t* data, std::size_t size) override
    {
        std::cout << "Tcp received " << size << " bytes: ";

        for (std::size_t i = 0; i < size; ++i) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') 
                << static_cast<int>(data[i]) << ' ';
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
       std::cerr << "Tcp client error: " << ec.message() << std::endl;
   });

   tcp_client.connect("127.0.0.1", 5000);

   std::cout << "Netforge tcp_client started on port " << tcp_client.port() << std::endl;

   io.run();

   return 0;
}