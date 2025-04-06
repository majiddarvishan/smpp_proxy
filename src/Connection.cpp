#include "Connection.hpp"
#include <spdlog/spdlog.h>

Connection::Connection(
    std::shared_ptr<boost::asio::ip::tcp::socket> client,
    std::shared_ptr<boost::asio::ip::tcp::socket> server)
    : client_socket_(client)
    , server_socket_(server)
{}

void Connection::start()
{
    start_forwarding(client_socket_, server_socket_);
    start_forwarding(server_socket_, client_socket_);
}

void Connection::start_forwarding(
    std::shared_ptr<boost::asio::ip::tcp::socket> from,
    std::shared_ptr<boost::asio::ip::tcp::socket> to)
{
    auto buffer = std::make_shared<std::vector<uint8_t>>(8192);
    auto self = shared_from_this();

    from->async_read_some(boost::asio::buffer(*buffer),
        [self, from, to, buffer](const boost::system::error_code& error, std::size_t length) {
            if (!error)
            {
                boost::asio::async_write(*to, boost::asio::buffer(*buffer, length),
                    [self, from, to, buffer](const boost::system::error_code& write_error, std::size_t) {
                        if (!write_error)
                        {
                            self->start_forwarding(from, to);
                        }
                        else
                        {
                            spdlog::warn("Write error: {}", write_error.message());
                            self->safely_close(from);
                            self->safely_close(to);
                        }
                    });
            }
            else
            {
                spdlog::warn("Read error: {}", error.message());
                self->safely_close(from);
                self->safely_close(to);
            }
        });
}

void Connection::safely_close(std::shared_ptr<boost::asio::ip::tcp::socket> sock)
{
    if (sock && sock->is_open())
    {
        boost::system::error_code ec;
        sock->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        if (!ec)
            sock->close(ec);
        else
            spdlog::warn("Read error: {}", ec.message());
    }
}
