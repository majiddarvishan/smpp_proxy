// #include "smpp_proxy.hpp"
// #include <spdlog/spdlog.h>

// using namespace boost::asio;

// smpp_proxy::smpp_proxy(io_context& acceptor_io,
//                        IOContextPool& context_pool,
//                        BufferPool& buffer_pool,
//                        const std::vector<std::string>& upstream_hosts,
//                        int upstream_port)
//     : acceptor_(acceptor_io, ip::tcp::endpoint(ip::tcp::v4(), 4000))
//     , acceptor_io_(acceptor_io)
//     , context_pool_(context_pool)
//     , buffer_pool_(buffer_pool)
//     , upstream_hosts_(upstream_hosts)
//     , upstream_port_(upstream_port)
//     , round_robin_index_(0)
// {
//     acceptor_.set_option(ip::tcp::acceptor::reuse_address(true));
//     spdlog::info("SMPP Proxy started on port 4000");
//     start_accept();
// }

// std::string smpp_proxy::get_next_upstream_host() {
//     return upstream_hosts_[round_robin_index_.fetch_add(1) % upstream_hosts_.size()];
// }

// void smpp_proxy::start_accept() {
//     auto client_socket = std::make_shared<ip::tcp::socket>(acceptor_io_);
//     acceptor_.async_accept(*client_socket, [this, client_socket](const boost::system::error_code& ec) {
//         if (!ec) {
//             client_socket->set_option(ip::tcp::no_delay(true));
//             client_socket->set_option(socket_base::keep_alive(true));

//             // Pick an io_context from the pool for this connection
//             boost::asio::io_context& conn_io = context_pool_.get_io_context();

//             handle_connection(client_socket, conn_io);
//         } else {
//             spdlog::error("Accept error: {}", ec.message());
//         }
//         start_accept(); // Accept next connection
//     });
// }

// void smpp_proxy::handle_connection(std::shared_ptr<ip::tcp::socket> client_socket, boost::asio::io_context& io)
// {
//     auto server_socket = std::make_shared<ip::tcp::socket>(io);
//     std::string upstream_host = get_next_upstream_host();
//     ip::tcp::endpoint endpoint(ip::make_address(upstream_host), upstream_port_);

//     server_socket->async_connect(endpoint, [self = shared_from_this(), client_socket, server_socket, upstream_host](const boost::system::error_code& ec) {
//         if (!ec) {
//             server_socket->set_option(ip::tcp::no_delay(true));
//             server_socket->set_option(socket_base::keep_alive(true));
//             self->start_forwarding(client_socket, server_socket);
//             self->start_forwarding(server_socket, client_socket);
//         } else {
//             spdlog::error("Connect error to {}: {}", upstream_host, ec.message());
//             client_socket->close();
//         }
//     });
// }

// void smpp_proxy::start_forwarding(std::shared_ptr<boost::asio::ip::tcp::socket> from, std::shared_ptr<boost::asio::ip::tcp::socket> to)
// {
//     auto buffer = std::make_shared<std::vector<uint8_t>>(8192);
//     from->async_read_some(boost::asio::buffer(*buffer),
//         [this, from, to, buffer](const boost::system::error_code& error, size_t length)
//         {
//             if (!error)
//             {
//                 boost::asio::async_write(*to, boost::asio::buffer(buffer->data(), length),
//                     [this, from, to, buffer](const boost::system::error_code& write_error, size_t)
//                     {
//                         if (!write_error)
//                         {
//                             start_forwarding(from, to); // continue forwarding
//                         }
//                         else
//                         {
//                             spdlog::warn("Write error: {}", write_error.message());
//                             shutdown_both(from, to);
//                         }
//                     });
//             }
//             else
//             {
//                 spdlog::warn("Read error: {}", error.message());
//                 shutdown_both(from, to);
//             }
//         });
// }

// void smpp_proxy::shutdown_both(std::shared_ptr<boost::asio::ip::tcp::socket> a, std::shared_ptr<boost::asio::ip::tcp::socket> b)
// {
//     boost::system::error_code ec;

//     if (a->is_open())
//     {
//         a->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
//         a->close(ec);
//     }

//     if (b->is_open())
//     {
//         b->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
//         b->close(ec);
//     }
// }

// #include "SmppProxy.hpp"

// using boost::asio::ip::tcp;

// SmppProxy::SmppProxy(IOContextPool& io_pool, const std::vector<std::string>& upstream_hosts, int upstream_port)
//     : acceptor_(io_pool.get_io_context(), tcp::endpoint(tcp::v4(), 4000)),
//       io_pool_(io_pool),
//       upstream_hosts_(upstream_hosts),
//       upstream_port_(upstream_port),
//       round_robin_index_(0),
//       buffer_pool_(8192, 1024)
// {
//     spdlog::info("SMPP Proxy started on port 4000");
// }

// void SmppProxy::start()
// {
//     start_accept();
// }

// void SmppProxy::start_accept()
// {
//     auto client_socket = std::make_shared<tcp::socket>(acceptor_.get_executor());
//     acceptor_.async_accept(*client_socket, [self = shared_from_this(), client_socket](const boost::system::error_code& error) {
//         if (!error)
//         {
//             spdlog::info("New client connected");
//             client_socket->set_option(tcp::no_delay(true));
//             self->handle_connection(client_socket);
//         }
//         else
//         {
//             spdlog::error("Accept error: {}", error.message());
//         }
//         self->start_accept();
//     });
// }

// std::string SmppProxy::get_next_upstream_host()
// {
//     return upstream_hosts_[round_robin_index_++ % upstream_hosts_.size()];
// }

// void SmppProxy::handle_connection(std::shared_ptr<tcp::socket> client_socket)
// {
//     auto server_socket = std::make_shared<tcp::socket>(io_pool_.get_io_context());
//     std::string upstream_host = get_next_upstream_host();

//     spdlog::info("Connecting to upstream host: {}", upstream_host);

//     auto resolver = std::make_shared<tcp::resolver>(server_socket->get_executor());
//     resolver->async_resolve(upstream_host, std::to_string(upstream_port_),
//         [self = shared_from_this(), client_socket, server_socket, resolver](const boost::system::error_code& ec, tcp::resolver::results_type results) {
//             if (!ec)
//             {
//                 boost::asio::async_connect(*server_socket, results,
//                     [self, client_socket, server_socket](const boost::system::error_code& ec2, const tcp::endpoint&) {
//                         if (!ec2)
//                         {
//                             server_socket->set_option(tcp::no_delay(true));
//                             spdlog::info("Connected to upstream server");
//                             self->start_forwarding(client_socket, server_socket);
//                             self->start_forwarding(server_socket, client_socket);
//                             auto conn = std::make_shared<Connection>(client_socket, server_socket);
//     conn->start();

//                         }
//                         else
//                         {
//                             spdlog::error("Connection to upstream failed: {}", ec2.message());
//                         }
//                     });
//             }
//             else
//             {
//                 spdlog::error("Resolve failed: {}", ec.message());
//             }
//         });
// }

// void SmppProxy::start_forwarding(
//     std::shared_ptr<tcp::socket> from,
//     std::shared_ptr<tcp::socket> to)
// {
//     auto buffer = std::make_shared<std::vector<uint8_t>>(8192);
//     from->async_read_some(boost::asio::buffer(*buffer),
//         [this, from, to, buffer](const boost::system::error_code& error, std::size_t length) {
//             if (!error)
//             {
//                 boost::asio::async_write(*to, boost::asio::buffer(*buffer, length),
//                     [this, from, to, buffer](const boost::system::error_code& write_error, std::size_t) {
//                         if (!write_error)
//                         {
//                             start_forwarding(from, to);
//                         }
//                         else
//                         {
//                             spdlog::warn("Write error: {}", write_error.message());
//                             safely_close(from);
//                             safely_close(to);
//                         }
//                     });
//             }
//             else
//             {
//                 spdlog::warn("Read error: {}", error.message());
//                 safely_close(from);
//                 safely_close(to);
//             }
//         });
// }

// void SmppProxy::safely_close(std::shared_ptr<boost::asio::ip::tcp::socket> sock)
// {
//     if (sock && sock->is_open())
//     {
//         boost::system::error_code ec;
//         sock->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
//         if (!ec)
//             sock->close(ec);
//         else
//             spdlog::warn("Read error: {}", ec.message());
//     }
// }

#include "SmppProxy.hpp"
#include "Connection.hpp"

#include <spdlog/spdlog.h>

using boost::asio::ip::tcp;

SmppProxy::SmppProxy(IOContextPool& io_pool,
    const std::vector<std::string>& upstream_hosts,
    int upstream_port,
    int listen_port)
: acceptor_(io_pool.get_io_context(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), listen_port))
, io_pool_(io_pool)
, upstream_hosts_(upstream_hosts)
, upstream_port_(upstream_port)
, round_robin_index_(0)
{
    spdlog::info("SMPP Proxy started on port {}", listen_port);
}

void SmppProxy::start()
{
    start_accept();
}

std::string SmppProxy::get_next_upstream_host()
{
    return upstream_hosts_[round_robin_index_.fetch_add(1, std::memory_order_relaxed) % upstream_hosts_.size()];
}

void SmppProxy::start_accept()
{
    auto client_socket = std::make_shared<tcp::socket>(acceptor_.get_executor());
    acceptor_.async_accept(*client_socket, [self = shared_from_this(), client_socket](const boost::system::error_code& error) {
        if (!error)
        {
            spdlog::info("New connection accepted");
            client_socket->set_option(tcp::no_delay(true));
            self->handle_connection(client_socket);
        }
        else
        {
            spdlog::error("Accept error: {}", error.message());
        }
        self->start_accept();
    });
}

void SmppProxy::handle_connection(std::shared_ptr<tcp::socket> client_socket)
{
    std::string upstream_host = get_next_upstream_host();

    auto server_socket = std::make_shared<tcp::socket>(io_pool_.get_io_context());
    auto resolver = std::make_shared<tcp::resolver>(io_pool_.get_io_context());

    resolver->async_resolve(
        upstream_host,
        std::to_string(upstream_port_),
        [self = shared_from_this(), client_socket, server_socket, resolver, upstream_host](
            const boost::system::error_code& error,
            tcp::resolver::results_type endpoints) {
            if (!error)
            {
                boost::asio::async_connect(
                    *server_socket,
                    endpoints,
                    [self, client_socket, server_socket, upstream_host](const boost::system::error_code& conn_error, const tcp::endpoint&) {
                        if (!conn_error)
                        {
                            server_socket->set_option(tcp::no_delay(true));
                            spdlog::info("Connected to upstream SMPP server: {}", upstream_host);

                            auto connection = std::make_shared<Connection>(client_socket, server_socket);
                            connection->start();
                        }
                        else
                        {
                            spdlog::error("Failed to connect to upstream SMPP server {}: {}", upstream_host, conn_error.message());
                        }
                    });
            }
            else
            {
                spdlog::error("Resolve failed for {}: {}", upstream_host, error.message());
            }
        });
}
