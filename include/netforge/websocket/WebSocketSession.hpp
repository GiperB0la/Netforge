#pragma once
#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>
#include <string_view>
#include <vector>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "../Session.hpp"

namespace netforge
{
    class WebSocketSession : public Session
    {
    public:
        explicit WebSocketSession(boost::asio::ip::tcp::socket socket);
        ~WebSocketSession() override = default;
        WebSocketSession(const WebSocketSession&) = delete;
        WebSocketSession& operator=(const WebSocketSession&) = delete;

    public:
        void send(std::string message);
        void send(std::vector<std::uint8_t> data);

    protected:
        void on_start() final;
        void on_stop() final;
        void on_error(const boost::system::error_code& ec) final;

        virtual void on_connected() {}
        virtual void on_message(std::string_view message) = 0;
        virtual void on_binary(const std::vector<std::uint8_t>& data) {}
        virtual void on_disconnected() {}

    private:
        struct Message {
            std::string data;
            bool binary = false;
        };

    private:
        void do_accept();
        void do_read();
        void do_write();

    private:
        boost::beast::websocket::stream<boost::asio::ip::tcp::socket> websocket_;

        boost::beast::flat_buffer read_buffer_;
        std::deque<Message> write_queue_;

    private:
        static constexpr std::size_t MAX_MESSAGE_SIZE = 1024 * 1024;
    };
}