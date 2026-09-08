#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <functional>
#include <atomic>
#include <utility>

#include <boost/asio.hpp>

namespace netforge
{
    template<class Session>
    class Client
    {
    public:
        explicit Client(boost::asio::io_context& io)
            : io_(io), resolver_(io) {
        }

        ~Client() {
            disconnect();
        }

        Client(const Client&) = delete;
        Client& operator=(const Client&) = delete;

    public:
        void connect(const std::string& host, uint16_t port) {
            if (connecting_ || connected_) {
                return;
            }

            connecting_ = true;
            host_ = host;
            port_ = port;

            resolver_.async_resolve(host, std::to_string(port),
                [this](const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::results_type results) {
                    if (ec) {
                        connecting_ = false;
                        handle_error(ec);
                        return;
                    }

                    do_connect(std::move(results));
                }
            );
        }

        void disconnect() {
            connecting_ = false;
            resolver_.cancel();

            if (socket_) {
                boost::system::error_code ec;
                socket_->cancel(ec);
                socket_->close(ec);
                socket_.reset();
            }

            if (session_) {
                session_->stop(false);
                session_.reset();
            }

            connected_ = false;
        }

    public:
        bool connected() const {
            return connected_.load();
        }

        bool connecting() const {
            return connecting_.load();
        }

        Session* session() {
            return session_.get();
        }

        const Session* session() const {
            return session_.get();
        }

        const std::string& host() const {
            return host_;
        }

        uint16_t port() const {
            return port_;
        }

    public:
        void set_error_handler(std::function<void(const boost::system::error_code&)> handler) {
            error_handler_ = std::move(handler);
        }

    private:
        void do_connect(boost::asio::ip::tcp::resolver::results_type results)
        {
            socket_ = std::make_shared<boost::asio::ip::tcp::socket>(io_);

            boost::asio::async_connect(*socket_, results,
                [this](const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint&) {
                    connecting_ = false;

                    if (ec) {
                        if (ec != boost::asio::error::operation_aborted) {
                            handle_error(ec);
                        }

                        socket_.reset();
                        return;
                    }

                    auto session = std::make_shared<Session>(std::move(*socket_));
                    socket_.reset();

                    auto weak_session = std::weak_ptr<Session>(session);

                    session->set_stop_handler([this, weak_session]() {
                        if (auto session = weak_session.lock()) {
                            if (session_ == session) {
                                connected_ = false;
                                session_.reset();
                            }
                        }
                    });

                    session_ = std::move(session);
                    connected_ = true;

                    session_->start();
                }
            );
        }

        void handle_error(const boost::system::error_code& ec) {
            if (error_handler_) {
                error_handler_(ec);
            }
        }

    private:
        boost::asio::io_context& io_;
        boost::asio::ip::tcp::resolver resolver_;
        std::shared_ptr<boost::asio::ip::tcp::socket> socket_;

        std::shared_ptr<Session> session_;

        std::string host_;
        uint16_t port_{ 0 };

        std::atomic_bool connecting_{ false };
        std::atomic_bool connected_{ false };

        std::function<void(const boost::system::error_code&)> error_handler_;
    };
}