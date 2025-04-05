// #pragma once
// #include <boost/asio.hpp>
// #include <thread>
// #include <vector>
// #include <memory>
// #include <atomic>

// class IOContextPool {
// public:
//     explicit IOContextPool(std::size_t pool_size)
//         : next_io_context_(0)
//     {
//         for (std::size_t i = 0; i < pool_size; ++i) {
//             auto io_context = std::make_shared<boost::asio::io_context>();
//             io_contexts_.push_back(io_context);
//             work_.emplace_back(boost::asio::make_work_guard(*io_context));
//         }
//     }

//     void run() {
//         for (auto& io : io_contexts_) {
//             threads_.emplace_back([io]() { io->run(); });
//         }
//     }

//     void join() {
//         for (auto& t : threads_) {
//             if (t.joinable()) t.join();
//         }
//     }

//     boost::asio::io_context& get_io_context() {
//         return *io_contexts_[next_io_context_++ % io_contexts_.size()];
//     }

// private:
//     std::vector<std::shared_ptr<boost::asio::io_context>> io_contexts_;
//     std::vector<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> work_;
//     std::vector<std::thread> threads_;
//     std::atomic<size_t> next_io_context_;
// };


#pragma once
#include <boost/asio.hpp>
#include <vector>
#include <thread>
#include <atomic>

class IOContextPool
{
public:
    explicit IOContextPool(std::size_t pool_size);

    void run();
    void stop();
    void join(); 

    boost::asio::io_context& get_io_context();

private:
    std::vector<std::shared_ptr<boost::asio::io_context>> io_contexts_;
    std::vector<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> work_;
    std::vector<std::thread> threads_;
    std::atomic<std::size_t> next_io_context_;
};
