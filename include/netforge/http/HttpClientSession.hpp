#pragma once
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "../Session.hpp"

namespace netforge
{
    class HttpClientSession : public Session
    {
    public:
        explicit HttpClientSession(boost::asio::ip::tcp::socket socket);
        ~HttpClientSession() override = default;
        HttpClientSession(const HttpClientSession&) = delete;
        HttpClientSession& operator=(const HttpClientSession&) = delete;

    public:
        void send(boost::beast::http::request<boost::beast::http::string_body> request);

    protected:
        void on_start() final;
        void on_stop() final;
        void on_error(const boost::system::error_code& ec) final;

        virtual void on_connected() {}
        virtual void on_response(const boost::beast::http::response<boost::beast::http::string_body>& response) = 0;
        virtual void on_disconnected() {}

    private:
        void do_read();
        void do_write();

    private:
        boost::beast::flat_buffer read_buffer_;
        std::optional<boost::beast::http::response_parser<boost::beast::http::string_body>> response_parser_;
        std::deque<boost::beast::http::request<boost::beast::http::string_body>> write_queue_;

    private:
        static constexpr std::uint32_t MAX_RESPONSE_HEADER_SIZE = 16 * 1024;
        static constexpr std::size_t MAX_RESPONSE_BODY_SIZE = 1024 * 1024;
    };
}