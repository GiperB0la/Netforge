#include "../include/netforge/HttpSession.hpp"

namespace netforge
{
    HttpSession::HttpSession(boost::asio::ip::tcp::socket socket)
        : Session(std::move(socket))
    {

    }

    void HttpSession::send(boost::beast::http::response<boost::beast::http::string_body> response)
    {
        auto self = std::static_pointer_cast<HttpSession>(shared_from_this());

        boost::asio::post(strand_, [self, response = std::move(response)]() mutable {
            if (!self->running()) return;

            const bool idle = self->write_queue_.empty();

            self->write_queue_.push_back(std::move(response));

            if (idle) {
                self->do_write();
            }
        });
    }

    void HttpSession::on_start()
    {
        on_connected();
        do_read();
    }

    void HttpSession::on_stop()
    {
        write_queue_.clear();
        on_disconnected();
    }

    void HttpSession::on_error(const boost::system::error_code& ec)
    {
        if (ec == boost::asio::error::operation_aborted) {
            return;
        }

        if (ec == boost::beast::http::error::end_of_stream ||
            ec == boost::beast::http::error::header_limit ||
            ec == boost::beast::http::error::body_limit) {
            stop();
            return;
        }

        Session::on_error(ec);
    }

    void HttpSession::do_read()
    {
        auto self = std::static_pointer_cast<HttpSession>(shared_from_this());

        request_parser_.emplace();
        request_parser_->header_limit(MAX_REQUEST_HEADER_SIZE);
        request_parser_->body_limit(MAX_REQUEST_BODY_SIZE);

        boost::beast::http::async_read(socket(), read_buffer_, *request_parser_,
            boost::asio::bind_executor(strand_,
                [self](boost::system::error_code ec, std::size_t) {
                    if (ec) {
                        self->on_error(ec);
                        return;
                    }

                    self->on_request(self->request_parser_->get());
                }
            )
        );
    }

    void HttpSession::do_write()
    {
        if (write_queue_.empty()) return;

        auto self = std::static_pointer_cast<HttpSession>(shared_from_this());

        const bool keep_alive = write_queue_.front().keep_alive();

        boost::beast::http::async_write(socket(), write_queue_.front(),
            boost::asio::bind_executor(strand_,
                [self, keep_alive](boost::system::error_code ec, std::size_t) {
                    if (ec) {
                        self->on_error(ec);
                        return;
                    }

                    self->write_queue_.pop_front();

                    if (!keep_alive) {
                        self->stop();
                        return;
                    }

                    if (self->running()) {
                        self->do_read();
                    }
                }
            )
        );
    }
}