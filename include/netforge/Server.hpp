#pragma once
#include <cstdint>
#include <memory>
#include <utility>
#include <functional>
#include <atomic>
#include <unordered_set>
#include <vector>

#include <boost/asio.hpp>

namespace netforge
{
    template<class Session>
    class Server
    {
    public:
        Server(boost::asio::io_context& io, uint16_t port)
            : acceptor_(io), port_(port) {
        }

        ~Server() {
            stop();
        }

        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;

    public:
        void start() {
            if (running_) return;

            boost::system::error_code ec;

            acceptor_.open(boost::asio::ip::tcp::v4(), ec);
            if (ec) {
                return fail_start(ec);
            }

            acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);

            if (ec) {
                return fail_start(ec);
            }

            acceptor_.bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_), ec);

            if (ec) {
                return fail_start(ec);
            }

            acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);

            if (ec) {
                return fail_start(ec);
            }

            running_ = true;
            do_accept();
        }

        void stop() {
            if (!running_) return;

            running_ = false;

            boost::system::error_code ec;
            acceptor_.cancel(ec);
            acceptor_.close(ec);

            for (auto& session : sessions_) {
                session->stop(false);
            }

            sessions_.clear();
        }

        bool running() const {
            return running_.load();
        }

        uint16_t port() const {
            return port_;
        }

        std::size_t session_count() const {
            return sessions_.size();
        }

        void set_error_handler(std::function<void(const boost::system::error_code&)> handler) {
            error_handler_ = std::move(handler);
        }

    private:
        void do_accept() {
            acceptor_.async_accept(
                [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
                {
                    if (!ec) {
                        auto session = std::make_shared<Session>(std::move(socket));
                        auto weak_session = std::weak_ptr<Session>(session);

                        session->set_stop_handler([this, weak_session]() {
                            if (auto session = weak_session.lock()) {
                                sessions_.erase(session);
                            }
                        });

                        sessions_.insert(session);
                        session->start();
                    }
                    else if (ec != boost::asio::error::operation_aborted) {
                        handle_error(ec);
                    }

                    if (running_) {
                        do_accept();
                    }
                }
            );
        }

        void handle_error(const boost::system::error_code& ec) {
            if (error_handler_) {
                error_handler_(ec);
            }
        }

        void fail_start(const boost::system::error_code& ec) {
            boost::system::error_code ignored;
            acceptor_.close(ignored);

            handle_error(ec);
        }

    private:
        boost::asio::ip::tcp::acceptor acceptor_;

        std::unordered_set<std::shared_ptr<Session>> sessions_;

        std::atomic_bool running_{ false };
        std::function<void(const boost::system::error_code&)> error_handler_;

        uint16_t port_{ 0 };
    };
}