#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <vector>
#include <span>

#include <boost/asio.hpp>

#include "Session.hpp"

namespace netforge
{
    class TcpSession : public Session
    {
    public:
        explicit TcpSession(boost::asio::ip::tcp::socket socket);
        ~TcpSession() override = default;
        TcpSession(const TcpSession&) = delete;
        TcpSession& operator=(const TcpSession&) = delete;

    public:
        void send(std::vector<std::uint8_t> data);
        void send(std::span<const std::uint8_t> data);

    protected:
        void on_start() final;
        void on_stop() final;
        void on_error(const boost::system::error_code& ec) final;

        virtual void on_connected() {}
        virtual void on_receive(const std::uint8_t* data, std::size_t size) = 0;
        virtual void on_disconnected() {}

    private:
        void do_read();
        void do_write();

    private:
        static constexpr std::size_t READ_BUFFER_SIZE = 8192;

    private:
        std::array<std::uint8_t, READ_BUFFER_SIZE> read_buffer_{};
        std::deque<std::vector<std::uint8_t>> write_queue_;
    };
}