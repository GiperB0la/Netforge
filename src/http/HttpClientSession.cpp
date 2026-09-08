#include "../../include/netforge/http/HttpClientSession.hpp"

namespace netforge
{
    HttpClientSession::HttpClientSession(boost::asio::ip::tcp::socket socket)
        : Session(std::move(socket))
    {

    }

    void HttpClientSession::send(
        boost::beast::http::request<boost::beast::http::string_body> request)
    {
        auto self = std::static_pointer_cast<HttpClientSession>(shared_from_this());

        boost::asio::post(strand_, [self, request = std::move(request)]() mutable {
            if (!self->running()) return;

            const bool idle = self->write_queue_.empty();

            self->write_queue_.push_back(std::move(request));

            if (idle) {
                self->do_write();
            }
        });
    }

    void HttpClientSession::on_start()
    {
        on_connected();
    }

    void HttpClientSession::on_stop()
    {
        write_queue_.clear();
        on_disconnected();
    }

    void HttpClientSession::on_error(const boost::system::error_code& ec)
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

    void HttpClientSession::do_read()
    {
        auto self = std::static_pointer_cast<HttpClientSession>(shared_from_this());

        response_parser_.emplace();
        response_parser_->header_limit(MAX_RESPONSE_HEADER_SIZE);
        response_parser_->body_limit(MAX_RESPONSE_BODY_SIZE);

        boost::beast::http::async_read(socket(), read_buffer_, *response_parser_,
            boost::asio::bind_executor(strand_,
                [self](boost::system::error_code ec, std::size_t) {
                    if (ec) {
                        self->on_error(ec);
                        return;
                    }

                    self->on_response(self->response_parser_->get());

                    if (self->running() &&
                        self->response_parser_->get().keep_alive()) {
                        self->do_read();
                    }
                    else {
                        self->stop();
                    }
                }
            )
        );
    }

    void HttpClientSession::do_write()
    {
        if (write_queue_.empty()) return;

        auto self = std::static_pointer_cast<HttpClientSession>(shared_from_this());

        boost::beast::http::async_write(socket(), write_queue_.front(),
            boost::asio::bind_executor(strand_,
                [self](boost::system::error_code ec, std::size_t) {
                    if (ec) {
                        self->on_error(ec);
                        return;
                    }

                    self->write_queue_.pop_front();

                    if (!self->running()) {
                        return;
                    }

                    self->do_read();
                }
            )
        );
    }
}