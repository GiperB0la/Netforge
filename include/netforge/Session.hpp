#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <atomic>
#include <functional>

#include <boost/asio.hpp>

namespace netforge 
{
    class Session : public std::enable_shared_from_this<Session>
    {
    public:
        explicit Session(boost::asio::ip::tcp::socket socket);
        virtual ~Session() = default;
        Session(const Session&) = delete;
        Session& operator=(const Session&) = delete;

    public:
        void start();
        void stop(bool notify = true);
        bool running() const;

        const boost::asio::ip::tcp::endpoint& remote_endpoint() const;
        std::string address() const;
        uint16_t port() const;

        void set_stop_handler(std::function<void()> handler);

    protected:
        virtual void on_start() = 0;
        virtual void on_stop() = 0;
        virtual void on_error(const boost::system::error_code& ec);

        boost::asio::ip::tcp::socket& socket();
        boost::asio::any_io_executor executor() const;

    private:
        void close();

    protected:
        boost::asio::ip::tcp::socket socket_;
        boost::asio::ip::tcp::endpoint remote_endpoint_;
        boost::asio::strand<boost::asio::io_context::executor_type> strand_;

    private:
        std::atomic_bool running_{ false };
        std::function<void()> stop_handler_;
    };
}