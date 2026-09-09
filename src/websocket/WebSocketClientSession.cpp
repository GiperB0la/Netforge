#include "../../include/netforge/websocket/WebSocketClientSession.hpp"

namespace netforge
{
    WebSocketClientSession::WebSocketClientSession(boost::asio::ip::tcp::socket socket)
        : Session(std::move(socket)), websocket_(std::move(this->socket()))
    {

    }

    void WebSocketClientSession::set_handshake(std::string host, std::string target)
    {
        host_ = std::move(host);
        target_ = std::move(target);
    }

    void WebSocketClientSession::send(std::string message)
    {
        auto self = std::static_pointer_cast<WebSocketClientSession>(shared_from_this());

        boost::asio::post(strand_,
            [self, message = std::move(message)]() mutable {
                if (!self->running()) {
                    return;
                }

                const bool idle = self->write_queue_.empty();

                self->write_queue_.push_back({
                    std::move(message),
                    false
                });

                if (idle) {
                    self->do_write();
                }
            }
        );
    }

    void WebSocketClientSession::send(std::vector<std::uint8_t> data)
    {
        auto self = std::static_pointer_cast<WebSocketClientSession>(shared_from_this());

        boost::asio::post(strand_,
            [self, data = std::move(data)]() mutable {
                if (!self->running()) {
                    return;
                }

                const bool idle = self->write_queue_.empty();

                self->write_queue_.push_back({
                    std::string(
                        reinterpret_cast<const char*>(data.data()),
                        data.size()
                    ),
                    true
                });

                if (idle) {
                    self->do_write();
                }
            }
        );
    }

    void WebSocketClientSession::on_start()
    {
        websocket_.binary(false);
        websocket_.read_message_max(MAX_MESSAGE_SIZE);

        do_handshake();
    }

    void WebSocketClientSession::on_stop()
    {
        write_queue_.clear();
        on_disconnected();
    }

    void WebSocketClientSession::on_error(const boost::system::error_code& ec)
    {
        if (ec == boost::asio::error::operation_aborted) {
            return;
        }

        if (ec == boost::beast::websocket::error::closed) {
            stop();
            return;
        }

        Session::on_error(ec);
    }

    void WebSocketClientSession::do_handshake()
    {
        auto self = std::static_pointer_cast<WebSocketClientSession>(shared_from_this());

        websocket_.async_handshake(host_, target_,
            boost::asio::bind_executor(strand_,
                [self](boost::system::error_code ec) {
                    if (ec) {
                        self->on_error(ec);
                        return;
                    }

                    self->on_connected();
                    self->do_read();
                }
            )
        );
    }

    void WebSocketClientSession::do_read()
    {
        auto self = std::static_pointer_cast<WebSocketClientSession>(shared_from_this());

        websocket_.async_read(read_buffer_, boost::asio::bind_executor(strand_,
                [self](boost::system::error_code ec, std::size_t) {
                    if (ec) {
                        self->on_error(ec);
                        return;
                    }

                    if (self->websocket_.got_text()) {
                        auto data = boost::beast::buffers_to_string(
                            self->read_buffer_.data()
                        );

                        self->on_message(data);
                    }
                    else {
                        const auto buffer = self->read_buffer_.data();

                        std::vector<std::uint8_t> data(
                            boost::asio::buffers_begin(buffer),
                            boost::asio::buffers_end(buffer)
                        );

                        self->on_binary(data);
                    }

                    self->read_buffer_.consume(
                        self->read_buffer_.size()
                    );

                    if (self->running()) {
                        self->do_read();
                    }
                }
            )
        );
    }

    void WebSocketClientSession::do_write()
    {
        if (write_queue_.empty()) {
            return;
        }

        auto self = std::static_pointer_cast<WebSocketClientSession>(shared_from_this());

        auto& message = write_queue_.front();

        websocket_.binary(message.binary);

        websocket_.async_write(boost::asio::buffer(message.data), boost::asio::bind_executor(strand_,
                [self](boost::system::error_code ec, std::size_t) {
                    if (ec) {
                        self->on_error(ec);
                        return;
                    }

                    self->write_queue_.pop_front();

                    if (self->running() &&
                        !self->write_queue_.empty()) {
                        self->do_write();
                    }
                }
            )
        );
    }
}