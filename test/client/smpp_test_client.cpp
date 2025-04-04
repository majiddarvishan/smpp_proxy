#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

using boost::asio::ip::tcp;

class TestClient : public std::enable_shared_from_this<TestClient>
{
public:
    TestClient(boost::asio::io_context& io_context, const std::string& host, int port)
        : socket_(io_context), endpoint_(boost::asio::ip::make_address(host), port) {}

    void start()
    {
        auto self = shared_from_this();
        socket_.async_connect(endpoint_, [this, self](boost::system::error_code ec) {
            if (!ec)
            {
                std::string data = "TEST-SMPP-DATA";
                boost::asio::async_write(socket_, boost::asio::buffer(data),
                                         [this, self](boost::system::error_code ec, std::size_t) {
                                             if (!ec)
                                                 read_response();
                                             else
                                                 socket_.close();
                                         });
            }
        });
    }

private:
    void read_response()
    {
        auto self = shared_from_this();
        socket_.async_read_some(boost::asio::buffer(buffer_),
                                [this, self](boost::system::error_code ec, std::size_t length) {
                                    if (!ec)
                                    {
                                        std::cout << "Received: " << std::string(buffer_.data(), length) << std::endl;
                                    }
                                    socket_.close();
                                });
    }

    tcp::socket socket_;
    tcp::endpoint endpoint_;
    std::array<char, 1024> buffer_;
};

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        std::cout << "Usage: test_client <host> <port> <connection_count>\n";
        return 1;
    }

    std::string host = argv[1];
    int port = std::stoi(argv[2]);
    int connection_count = std::stoi(argv[3]);

    boost::asio::io_context io_context;
    std::vector<std::shared_ptr<TestClient>> clients;

    for (int i = 0; i < connection_count; ++i)
    {
        auto client = std::make_shared<TestClient>(io_context, host, port);
        clients.push_back(client);
        client->start();
    }

    std::vector<std::thread> threads;
    int thread_count = std::min(connection_count, 16);
    for (int i = 0; i < thread_count; ++i)
    {
        threads.emplace_back([&io_context]() {
            io_context.run();
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    return 0;
}
