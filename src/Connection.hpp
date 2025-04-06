#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <vector>

class Connection : public std::enable_shared_from_this<Connection>
{
public:
    Connection(std::shared_ptr<boost::asio::ip::tcp::socket> client,
               std::shared_ptr<boost::asio::ip::tcp::socket> server);

    void start();

private:
    std::shared_ptr<boost::asio::ip::tcp::socket> client_socket_;
    std::shared_ptr<boost::asio::ip::tcp::socket> server_socket_;

    void start_forwarding(std::shared_ptr<boost::asio::ip::tcp::socket> from,
                          std::shared_ptr<boost::asio::ip::tcp::socket> to);

    void safely_close(std::shared_ptr<boost::asio::ip::tcp::socket> sock);
};
