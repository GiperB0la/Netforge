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
        std::cout << "Http client connected: " << address() << ':' << port() << std::endl;
    }

    void on_request(const boost::beast::http::request<boost::beast::http::string_body>& request) override
    {
        boost::beast::http::response<boost::beast::http::string_body> response{
            boost::beast::http::status::ok,
            request.version()
        };

        response.keep_alive(request.keep_alive());
        response.set(boost::beast::http::field::content_type, "text/plain");
        response.body() = "Hello from Netforge!";
        response.prepare_payload();

        send(std::move(response));
    }

    void on_disconnected() override
    {
        std::cout << "Http client disconnected: " << address() << ':' << port() << std::endl;
    }
};

int main()
{
   boost::asio::io_context io;

   netforge::Server<MyHttpSession> http_server(io, 8080);

   http_server.set_error_handler([](const boost::system::error_code& ec) {
       std::cerr << "Http server error: " << ec.message() << std::endl;
   });

   http_server.start();

   std::cout << "Netforge http_server started on port " << http_server.port() << std::endl;

   io.run();

   return 0;
}