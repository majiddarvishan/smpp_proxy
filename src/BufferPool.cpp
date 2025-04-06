#include "BufferPool.hpp"

BufferPool::BufferPool(std::size_t buffer_size, std::size_t initial_pool_size)
    : buffer_size_(buffer_size)
{
    for (std::size_t i = 0; i < initial_pool_size; ++i)
    {
        pool_.emplace(std::make_shared<std::vector<uint8_t>>(buffer_size_));
    }
}

std::shared_ptr<std::vector<uint8_t>> BufferPool::acquire()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (pool_.empty())
    {
        return std::make_shared<std::vector<uint8_t>>(buffer_size_);
    }
    auto buf = pool_.top();
    pool_.pop();
    return buf;
}

void BufferPool::release(std::shared_ptr<std::vector<uint8_t>> buffer)
{
    std::lock_guard<std::mutex> lock(mutex_);
    pool_.push(buffer);
}  