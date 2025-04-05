#include "BufferPool.hpp"

BufferPool::BufferPool(std::size_t buffer_size, std::size_t max_pool_size)
    : buffer_size_(buffer_size), max_pool_size_(max_pool_size)
{
}

std::shared_ptr<std::vector<uint8_t>> BufferPool::acquire()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!pool_.empty())
    {
        auto buffer = pool_.front();
        pool_.pop();
        return buffer;
    }
    return std::make_shared<std::vector<uint8_t>>(buffer_size_);
}

void BufferPool::release(std::shared_ptr<std::vector<uint8_t>> buffer)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (pool_.size() < max_pool_size_)
    {
        buffer->clear();
        buffer->resize(buffer_size_);
        pool_.push(buffer);
    }
}
