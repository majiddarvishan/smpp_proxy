#pragma once

#include "BufferPool.hpp"

#include <boost/asio.hpp>
#include <memory>
#include <vector>

class Connection : public std::enable_shared_from_this<Connection>
{
public:
    Connection(std::shared_ptr<boost::asio::ip::tcp::socket> client,
               std::shared_ptr<boost::asio::ip::tcp::socket> server,
               BufferPool* buffer_pool);

    void start();

private:
    void start_forwarding(std::shared_ptr<boost::asio::ip::tcp::socket> from,
                          std::shared_ptr<boost::asio::ip::tcp::socket> to);
    void safely_close(std::shared_ptr<boost::asio::ip::tcp::socket> sock);

    std::shared_ptr<boost::asio::ip::tcp::socket> client_socket_;
    std::shared_ptr<boost::asio::ip::tcp::socket> server_socket_;
    BufferPool* buffer_pool_; // added buffer pool
};