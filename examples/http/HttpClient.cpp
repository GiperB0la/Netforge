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
        std::cout << "HTTP client connected: " << address() << ':' << port() << std::endl;

        boost::beast::http::request<boost::beast::http::string_body> request{
            boost::beast::http::verb::get,
            "/",
            11
        };

        request.set(boost::beast::http::field::host, address());
        request.set(boost::beast::http::field::user_agent, "Netforge");
        request.keep_alive(true);

        send(std::move(request));
    }

    void on_response(const boost::beast::http::response<boost::beast::http::string_body>& response) override
    {
        std::cout << "HTTP response:" << std::endl;
        std::cout << "Status: " << response.result_int() << std::endl;
        std::cout << "Body: " << response.body() << std::endl;
    }

    void on_disconnected() override
    {
        std::cout << "HTTP client disconnected: " << address() << ':' << port() << std::endl;
    }
};

int main()
{
    boost::asio::io_context io;

    netforge::Client<MyHttpClientSession> http_client(io);

    http_client.set_error_handler([](const boost::system::error_code& ec) {
        std::cerr << "HTTP client error: " << ec.message() << std::endl;
    });

    http_client.connect("127.0.0.1", 8080);

    io.run();

    return 0;
}