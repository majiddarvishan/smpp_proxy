#pragma once

#include <boost/asio.hpp>
#include <vector>
#include <atomic>
#include <memory>
#include <string>

#include "IOContextPool.hpp"
#include "BufferPool.hpp"

class SmppProxy : public std::enable_shared_from_this<SmppProxy>
{
public:
    SmppProxy(IOContextPool& io_pool,
              const std::vector<std::string>& upstream_hosts,
              int upstream_port,
              int listen_port = 4000);

    void start();

private:
    void start_accept();
    void handle_connection(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket);
    std::string get_next_upstream_host();

    boost::asio::ip::tcp::acceptor acceptor_;
    IOContextPool& io_pool_;
    std::vector<std::string> upstream_hosts_;
    int upstream_port_;
    std::atomic<size_t> current_upstream_index_;
    std::shared_ptr<BufferPool> buffer_pool_;
};
