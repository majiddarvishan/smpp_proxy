#include "smpp_proxy.hpp"
#include <spdlog/spdlog.h>

using namespace boost::asio;

smpp_proxy::smpp_proxy(io_context& acceptor_io,
                       IOContextPool& context_pool,
                       BufferPool& buffer_pool,
                       const std::vector<std::string>& upstream_hosts,
                       int upstream_port)
    : acceptor_(acceptor_io, ip::tcp::endpoint(ip::tcp::v4(), 4000))
    , acceptor_io_(acceptor_io)
    , context_pool_(context_pool)
    , buffer_pool_(buffer_pool)
    , upstream_hosts_(upstream_hosts)
    , upstream_port_(upstream_port)
    , round_robin_index_(0)
{
    acceptor_.set_option(ip::tcp::acceptor::reuse_address(true));
    spdlog::info("SMPP Proxy started on port 4000");
    start_accept();
}

std::string smpp_proxy::get_next_upstream_host() {
    return upstream_hosts_[round_robin_index_.fetch_add(1) % upstream_hosts_.size()];
}

void smpp_proxy::start_accept() {
    auto client_socket = std::make_shared<ip::tcp::socket>(acceptor_io_);
    acceptor_.async_accept(*client_socket, [this, client_socket](const boost::system::error_code& ec) {
        if (!ec) {
            client_socket->set_option(ip::tcp::no_delay(true));
            client_socket->set_option(socket_base::keep_alive(true));

            // Pick an io_context from the pool for this connection
            boost::asio::io_context& conn_io = context_pool_.get_io_context();

            handle_connection(client_socket, conn_io);
        } else {
            spdlog::error("Accept error: {}", ec.message());
        }
        start_accept(); // Accept next connection
    });
}

void smpp_proxy::handle_connection(std::shared_ptr<ip::tcp::socket> client_socket, boost::asio::io_context& io)
{
    auto server_socket = std::make_shared<ip::tcp::socket>(io);
    std::string upstream_host = get_next_upstream_host();
    ip::tcp::endpoint endpoint(ip::make_address(upstream_host), upstream_port_);

    server_socket->async_connect(endpoint, [self = shared_from_this(), client_socket, server_socket, upstream_host](const boost::system::error_code& ec) {
        if (!ec) {
            server_socket->set_option(ip::tcp::no_delay(true));
            server_socket->set_option(socket_base::keep_alive(true));
            self->start_forwarding(client_socket, server_socket);
            self->start_forwarding(server_socket, client_socket);
        } else {
            spdlog::error("Connect error to {}: {}", upstream_host, ec.message());
            client_socket->close();
        }
    });
}

void smpp_proxy::start_forwarding(std::shared_ptr<ip::tcp::socket> from, std::shared_ptr<ip::tcp::socket> to)
{
    auto buffer = buffer_pool_.acquire();
    from->async_read_some(boost::asio::buffer(*buffer), [this, from, to, buffer](const boost::system::error_code& ec, size_t length) {
        if (!ec) {
            async_write(*to, boost::asio::buffer(buffer->data(), length), [this, from, to, buffer](const boost::system::error_code& write_ec, size_t) {
                buffer_pool_.release(buffer);
                if (!write_ec) {
                    start_forwarding(from, to);
                } else {
                    from->close();
                    to->close();
                }
            });
        } else {
            from->close();
            to->close();
        }
    });
}
