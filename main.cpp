#include <boost/asio.hpp>
#include <spdlog/spdlog.h>

#include <atomic>
#include <condition_variable>
#include <thread>
#include <vector>

// using namespace boost::asio;

class smpp_proxy : public std::enable_shared_from_this<smpp_proxy>
{
  public:
    smpp_proxy(boost::asio::io_context& io, const std::vector<std::string>& upstream_hosts, int upstream_port)
        : acceptor_(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 4000))
        , upstream_hosts_(upstream_hosts)
        , upstream_port_(upstream_port)
        , io_(io)
        , round_robin_index_(0)
    {
        spdlog::info("SMPP Proxy started on port 4000");
        start_accept();
    }

  private:
    boost::asio::ip::tcp::acceptor acceptor_;
    std::vector<std::string> upstream_hosts_;
    int upstream_port_;
    boost::asio::io_context& io_;
    std::atomic<size_t> round_robin_index_;

    std::string get_next_upstream_host()
    {
        return upstream_hosts_[round_robin_index_.fetch_add(1, std::memory_order_relaxed) % upstream_hosts_.size()];
    }

    void start_accept()
    {
        auto client_socket = std::make_shared<boost::asio::ip::tcp::socket>(acceptor_.get_executor());
        acceptor_.async_accept(*client_socket, [this, client_socket](const boost::system::error_code& error) {
            if (!error)
            {
                spdlog::info("New connection accepted");
                client_socket->set_option(boost::asio::ip::tcp::no_delay(true));
                this->handle_connection(client_socket);
            }
            else
            {
                spdlog::error("Accept error: {}", error.message());
            }
            this->start_accept();
        });
    }

    void handle_connection(std::shared_ptr<boost::asio::ip::tcp::socket> client_socket)
    {
        auto server_socket = std::make_shared<boost::asio::ip::tcp::socket>(io_);
        std::string upstream_host = get_next_upstream_host();
        spdlog::info("Connecting to upstream SMPP server: {}", upstream_host);

        auto resolver = std::make_shared<boost::asio::ip::tcp::resolver>(io_);
        resolver->async_resolve(
            upstream_host,
            std::to_string(upstream_port_),
            [self = shared_from_this(), client_socket, server_socket, resolver, upstream_host](const boost::system::error_code& error, boost::asio::ip::tcp::resolver::results_type endpoints) {
                if (!error)
                {
                    async_connect(
                        *server_socket,
                        endpoints,
                        [self, client_socket, server_socket, upstream_host](const boost::system::error_code& conn_error, const boost::asio::ip::tcp::endpoint&) {
                            if (!conn_error)
                            {
                                if (server_socket->is_open())
                                {
                                    server_socket->set_option(boost::asio::ip::tcp::no_delay(true));
                                }
                                spdlog::info("Connected to upstream SMPP server: {}", upstream_host);
                                self->start_forwarding(client_socket, server_socket);
                                self->start_forwarding(server_socket, client_socket);
                            }
                            else
                            {
                                spdlog::error("Failed to connect to upstream SMPP server {}: {}", upstream_host, conn_error.message());
                            }
                        });
                }
                else
                {
                    spdlog::error("Failed to resolve {}: {}", upstream_host, error.message());
                }
            });
    }

    void start_forwarding(std::shared_ptr<boost::asio::ip::tcp::socket> from, std::shared_ptr<boost::asio::ip::tcp::socket> to)
    {
        auto buffer = std::make_shared<std::vector<uint8_t>>(8192);
        from->async_read_some(boost::asio::buffer(buffer->data(), buffer->size()), [this, from, to, buffer](const boost::system::error_code& error, size_t length) {
            if (!error)
            {
                boost::asio::async_write(*to, boost::asio::buffer(buffer->data(), length), [this, from, to, buffer](const boost::system::error_code& write_error, size_t) {
                    if (!write_error)
                    {
                        start_forwarding(from, to);
                    }
                    else
                    {
                        spdlog::error("Write error: {}", write_error.message());
                        from->close();
                        to->close();
                    }
                });
            }
            else
            {
                spdlog::error("Read error: {}", error.message());
                from->close();
                to->close();
            }
        });
    }
};

int main()
{
    try
    {
        boost::asio::io_context io;
        std::vector<std::string> upstream_hosts = { "127.0.0.1" };
        auto proxy = std::make_shared<smpp_proxy>(io, upstream_hosts, 3000);

        std::vector<std::thread> threads;
        const size_t thread_count = 4; // Fixed number of threads for better performance
        for (size_t i = 0; i < thread_count; ++i)
        {
            threads.emplace_back([&io]() { io.run(); });
        }

        for (auto& t : threads)
        {
            t.join();
        }
    }
    catch (std::exception& e)
    {
        spdlog::error("Exception: {}", e.what());
    }
    return 0;
}
