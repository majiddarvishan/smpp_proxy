// #pragma once
// #include <vector>
// #include <queue>
// #include <mutex>
// #include <memory>

// class BufferPool {
// public:
//     using Buffer = std::vector<uint8_t>;
//     using BufferPtr = std::shared_ptr<Buffer>;

//     BufferPtr acquire() {
//         std::lock_guard<std::mutex> lock(mutex_);
//         if (!pool_.empty()) {
//             auto buf = pool_.front();
//             pool_.pop();
//             return buf;
//         }
//         return std::make_shared<Buffer>(16384); // 16 KB
//     }

//     void release(BufferPtr buffer) {
//         std::lock_guard<std::mutex> lock(mutex_);
//         pool_.push(buffer);
//     }

// private:
//     std::queue<BufferPtr> pool_;
//     std::mutex mutex_;
// };

#pragma once

#include <queue>
#include <mutex>
#include <vector>
#include <memory>

class BufferPool
{
public:
    explicit BufferPool(std::size_t buffer_size, std::size_t max_pool_size);
    std::shared_ptr<std::vector<uint8_t>> acquire();
    void release(std::shared_ptr<std::vector<uint8_t>> buffer);

private:
    std::queue<std::shared_ptr<std::vector<uint8_t>>> pool_;
    std::mutex mutex_;
    std::size_t buffer_size_;
    std::size_t max_pool_size_;
};
