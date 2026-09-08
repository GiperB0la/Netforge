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
    class HttpSession : public Session
    {
    public:
        explicit HttpSession(boost::asio::ip::tcp::socket socket);
        ~HttpSession() override = default;
        HttpSession(const HttpSession&) = delete;
        HttpSession& operator=(const HttpSession&) = delete;

    public:
        void send(boost::beast::http::response<boost::beast::http::string_body> response);

    protected:
        void on_start() final;
        void on_stop() final;
        void on_error(const boost::system::error_code& ec) final;

        virtual void on_connected() {}
        virtual void on_request(const boost::beast::http::request<boost::beast::http::string_body>& request) = 0;
        virtual void on_disconnected() {}

    private:
        void do_read();
        void do_write();

    private:
        boost::beast::flat_buffer read_buffer_;
        std::optional<boost::beast::http::request_parser<boost::beast::http::string_body>> request_parser_;
        std::deque<boost::beast::http::response<boost::beast::http::string_body>> write_queue_;

    private:
        static constexpr std::uint32_t MAX_REQUEST_HEADER_SIZE = 16 * 1024;
        static constexpr std::size_t MAX_REQUEST_BODY_SIZE = 1024 * 1024;
    };
}