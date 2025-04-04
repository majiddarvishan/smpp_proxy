#include <spdlog/spdlog.h>
#include "buffer_pool.hpp"
#include "io_context_pool.hpp"
#include "smpp_proxy.hpp"

int main()
{
    try
    {
        const size_t thread_count = std::thread::hardware_concurrency();

        IOContextPool context_pool(thread_count);
        context_pool.run();

        BufferPool buffer_pool;
        std::vector<std::string> upstream_hosts = { "127.0.0.1" };

        // Only one acceptor using one context
        auto& main_io = context_pool.get_io_context();
        auto proxy = std::make_shared<smpp_proxy>(main_io, context_pool, buffer_pool, upstream_hosts, 3000);

        context_pool.join();
    }
    catch (const std::exception& ex)
    {
        spdlog::error("Fatal error: {}", ex.what());
    }

    return 0;
}
