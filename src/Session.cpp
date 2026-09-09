#include "../include/netforge/Session.hpp"

namespace netforge 
{
    Session::Session(boost::asio::ip::tcp::socket socket)
        : socket_(std::move(socket)),
        strand_(boost::asio::make_strand(static_cast<boost::asio::io_context&>(socket_.get_executor().context())))
    {
        boost::system::error_code ec;
        remote_endpoint_ = socket_.remote_endpoint(ec);

        if (ec) {
            remote_endpoint_ = {};
        }
    }

    void Session::start()
    {
        auto self = shared_from_this();

        boost::asio::dispatch(strand_, [self] {
            if (self->running_) return;

            self->running_ = true;
            self->on_start();
        });
    }

    void Session::stop(bool notify)
    {
        auto self = shared_from_this();

        boost::asio::dispatch(strand_, [self, notify] {
            if (!self->running_) return;

            self->running_ = false;
            self->on_stop();
            self->close();

            if (self->stop_handler_ && notify) {
                self->stop_handler_();
            }

            self->stop_handler_ = nullptr;
        });
    }

    void Session::close()
    {
        boost::system::error_code ec;

        socket_.cancel(ec);
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket_.close(ec);
    }

    void Session::set_stop_handler(std::function<void()> handler)
    {
        stop_handler_ = std::move(handler);
    }

    bool Session::running() const
    {
        return running_.load();
    }

    const boost::asio::ip::tcp::endpoint& Session::remote_endpoint() const
    {
        return remote_endpoint_;
    }

    std::string Session::address() const
    {
        return remote_endpoint_.address().to_string();
    }

    uint16_t Session::port() const
    {
        return remote_endpoint_.port();
    }

    void Session::on_error(const boost::system::error_code& ec)
    {
        if (ec == boost::asio::error::operation_aborted) return;

        stop();
    }

    boost::asio::ip::tcp::socket& Session::socket()
    {
        return socket_;
    }

    boost::asio::any_io_executor Session::executor() const
    {
        return strand_;
    }
}