#pragma once
#include <boost/asio.hpp>
#include "buffer_pool.hpp"
#include "io_context_pool.hpp"

class smpp_proxy : public std::enable_shared_from_this<smpp_proxy> {
public:
    smpp_proxy(boost::asio::io_context& acceptor_io,
               IOContextPool& context_pool,
               BufferPool& buffer_pool,
               const std::vector<std::string>& upstream_hosts,
               int upstream_port);

private:
    void start_accept();
    void handle_connection(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket, boost::asio::io_context& io);
    void start_forwarding(std::shared_ptr<boost::asio::ip::tcp::socket> from,
                          std::shared_ptr<boost::asio::ip::tcp::socket> to);

    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::io_context& acceptor_io_;
    IOContextPool& context_pool_;
    BufferPool& buffer_pool_;
    std::vector<std::string> upstream_hosts_;
    int upstream_port_;
    std::atomic<size_t> round_robin_index_;

    std::string get_next_upstream_host();
};
