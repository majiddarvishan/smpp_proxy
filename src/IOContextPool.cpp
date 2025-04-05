#include "IOContextPool.hpp"

IOContextPool::IOContextPool(std::size_t pool_size)
    : next_io_context_(0)
{
    for (std::size_t i = 0; i < pool_size; ++i)
    {
        auto ctx = std::make_shared<boost::asio::io_context>();
        io_contexts_.push_back(ctx);
        work_.emplace_back(boost::asio::make_work_guard(*ctx));
    }
}

void IOContextPool::run()
{
    for (auto& ctx : io_contexts_)
    {
        threads_.emplace_back([ctx]() { ctx->run(); });
    }
}

void IOContextPool::stop()
{
    for (auto& ctx : io_contexts_)
    {
        ctx->stop();
    }
}

void IOContextPool::join()
{
    for (auto& t : threads_)
    {
        if (t.joinable())
            t.join();
    }
}

boost::asio::io_context& IOContextPool::get_io_context()
{
    return *io_contexts_[next_io_context_++ % io_contexts_.size()];
}
