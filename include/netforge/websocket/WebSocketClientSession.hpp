#pragma once
#include <cstdint>
#include <deque>
#include <string>
#include <vector>
#include <utility>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include "../Session.hpp"

namespace netforge
{
    class WebSocketClientSession : public Session
    {
    public:
        WebSocketClientSession(boost::asio::ip::tcp::socket socket);

    public:
        void set_handshake(std::string host, std::string target = "/");
        void send(std::string message);
        void send(std::vector<std::uint8_t> data);

    protected:
        void on_start() override;
        void on_stop() override;
        void on_error(const boost::system::error_code& ec) override;

        virtual void on_connected() {}
        virtual void on_disconnected() {}
        virtual void on_message(const std::string& message) {}
        virtual void on_binary(const std::vector<std::uint8_t>& data) {}

    private:
        void do_handshake();
        void do_read();
        void do_write();

    private:
        struct WriteMessage {
            std::string data;
            bool binary;
        };

        boost::beast::websocket::stream<boost::asio::ip::tcp::socket> websocket_;
        boost::beast::flat_buffer read_buffer_;

        std::deque<WriteMessage> write_queue_;

        std::string host_;
        std::string target_;

    private:
        static constexpr std::size_t MAX_MESSAGE_SIZE = 16 * 1024 * 1024;
    };
}