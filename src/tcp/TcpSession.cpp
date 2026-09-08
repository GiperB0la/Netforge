#include "../../include/netforge/tcp/TcpSession.hpp"

namespace netforge
{
    TcpSession::TcpSession(boost::asio::ip::tcp::socket socket)
        : Session(std::move(socket))
    {

    }

    void TcpSession::send(std::span<const std::uint8_t> data)
    {
        auto self = std::static_pointer_cast<TcpSession>(shared_from_this());

        boost::asio::post(strand_, [self, data]() {
            if (!self->running()) return;

            const bool idle = self->write_queue_.empty();

            self->write_queue_.push_back(std::vector<std::uint8_t>(data.begin(), data.end()));

            if (idle) {
                self->do_write();
            }
        });
    }

    void TcpSession::on_start()
    {
        on_connected();
        do_read();
    }

    void TcpSession::on_stop()
    {
        write_queue_.clear();
        on_disconnected();
    }

    void TcpSession::on_error(const boost::system::error_code& ec)
    {
        Session::on_error(ec);
    }

    void TcpSession::do_read()
    {
        auto self = std::static_pointer_cast<TcpSession>(shared_from_this());

        socket().async_read_some(
            boost::asio::buffer(read_buffer_),
            boost::asio::bind_executor(strand_,
                [self](boost::system::error_code ec, std::size_t size) {
                    if (ec) {
                        self->on_error(ec);
                        return;
                    }

                    if (size > 0) {
                        self->on_receive(self->read_buffer_.data(), size);
                    }

                    if (self->running()) {
                        self->do_read();
                    }
                }
            )
        );
    }

    void TcpSession::do_write()
    {
        if (write_queue_.empty()) return;

        auto self = std::static_pointer_cast<TcpSession>(shared_from_this());

        boost::asio::async_write(
            socket(),
            boost::asio::buffer(write_queue_.front()),
            boost::asio::bind_executor(strand_,
                [self](boost::system::error_code ec, std::size_t) {
                    if (ec) {
                        self->on_error(ec);
                        return;
                    }

                    self->write_queue_.pop_front();

                    if (self->running()) {
                        self->do_write();
                    }
                }
            )
        );
    }
}