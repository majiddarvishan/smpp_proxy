#include <boost/asio.hpp>
#include <spdlog/spdlog.h>

#include <vector>
#include <thread>
#include <atomic>
#include <condition_variable>

using namespace boost::asio;

class SMPPProxy : public std::enable_shared_from_this<SMPPProxy> {
public:
    SMPPProxy(io_context& io, const std::vector<std::string>& upstream_hosts, int upstream_port)
        : acceptor_(io, ip::tcp::endpoint(ip::tcp::v4(), 4000)), upstream_hosts_(upstream_hosts), upstream_port_(upstream_port), io_(io), round_robin_index_(0) {
        spdlog::info("SMPP Proxy started on port 4000");
        startAccept();
    }

private:
    ip::tcp::acceptor acceptor_;
    std::vector<std::string> upstream_hosts_;
    int upstream_port_;
    io_context& io_;
    std::atomic<size_t> round_robin_index_;

    std::string getNextUpstreamHost() {
        return upstream_hosts_[round_robin_index_.fetch_add(1, std::memory_order_relaxed) % upstream_hosts_.size()];
    }

    void startAccept() {
        auto client_socket = std::make_shared<ip::tcp::socket>(acceptor_.get_executor());
        acceptor_.async_accept(*client_socket,
            [this, client_socket](const boost::system::error_code& error) {
                if (!error) {
                    spdlog::info("New connection accepted");
                    client_socket->set_option(ip::tcp::no_delay(true));
                    this->handleConnection(client_socket);
                } else {
                    spdlog::error("Accept error: {}", error.message());
                }
                this->startAccept();
            });
    }

    void handleConnection(std::shared_ptr<ip::tcp::socket> client_socket) {
        auto server_socket = std::make_shared<ip::tcp::socket>(io_);
        std::string upstream_host = getNextUpstreamHost();
        spdlog::info("Connecting to upstream SMPP server: {}", upstream_host);

        auto resolver = std::make_shared<ip::tcp::resolver>(io_);
        resolver->async_resolve(upstream_host, std::to_string(upstream_port_),
            [self = shared_from_this(), client_socket, server_socket, resolver, upstream_host](const boost::system::error_code& error, ip::tcp::resolver::results_type endpoints) {
                if (!error) {
                    async_connect(*server_socket, endpoints, [self, client_socket, server_socket, upstream_host](const boost::system::error_code& conn_error, const ip::tcp::endpoint&) {
                        if (!conn_error) {
                            if (server_socket->is_open()) {
                                server_socket->set_option(ip::tcp::no_delay(true));
                            }
                            spdlog::info("Connected to upstream SMPP server: {}", upstream_host);
                            self->startForwarding(client_socket, server_socket);
                            self->startForwarding(server_socket, client_socket);
                        } else {
                            spdlog::error("Failed to connect to upstream SMPP server {}: {}", upstream_host, conn_error.message());
                        }
                    });
                } else {
                    spdlog::error("Failed to resolve {}: {}", upstream_host, error.message());
                }
            });
    }

    void startForwarding(std::shared_ptr<ip::tcp::socket> from, std::shared_ptr<ip::tcp::socket> to) {
        auto buffer = std::make_shared<std::vector<uint8_t>>(8192);
        from->async_read_some(boost::asio::buffer(buffer->data(), buffer->size()),
            [this, from, to, buffer](const boost::system::error_code& error, size_t length) {
                if (!error) {
                    boost::asio::async_write(*to, boost::asio::buffer(buffer->data(), length),
                        [this, from, to, buffer](const boost::system::error_code& write_error, size_t) {
                            if (!write_error) {
                                startForwarding(from, to);
                            } else {
                                spdlog::error("Write error: {}", write_error.message());
                                from->close();
                                to->close();
                            }
                        });
                } else {
                    spdlog::error("Read error: {}", error.message());
                    from->close();
                    to->close();
                }
            });
    }
};

int main() {
    try {
        io_context io;
        std::vector<std::string> upstream_hosts = {"127.0.0.1"};
        auto proxy = std::make_shared<SMPPProxy>(io, upstream_hosts, 3000);

        std::vector<std::thread> threads;
        size_t thread_count = 4; // Fixed number of threads for better performance
        for (size_t i = 0; i < thread_count; ++i) {
            threads.emplace_back([&io]() { io.run(); });
        }

        for (auto& t : threads) {
            t.join();
        }
    } catch (std::exception& e) {
        spdlog::error("Exception: {}", e.what());
    }
    return 0;
}
