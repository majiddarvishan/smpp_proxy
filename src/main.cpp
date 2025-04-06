// #include <spdlog/spdlog.h>
// #include "buffer_pool.hpp"
// #include "io_context_pool.hpp"
// #include "smpp_proxy.hpp"

// int main()
// {
//     try
//     {
//         const size_t thread_count = std::thread::hardware_concurrency();

//         IOContextPool context_pool(thread_count);
//         context_pool.run();

//         BufferPool buffer_pool;
//         std::vector<std::string> upstream_hosts = { "127.0.0.1" };

//         // Only one acceptor using one context
//         auto& main_io = context_pool.get_io_context();
//         auto proxy = std::make_shared<smpp_proxy>(main_io, context_pool, buffer_pool, upstream_hosts, 3000);

//         context_pool.join();
//     }
//     catch (const std::exception& ex)
//     {
//         spdlog::error("Fatal error: {}", ex.what());
//     }

//     return 0;
// }

#include <spdlog/spdlog.h>
#include <boost/asio/signal_set.hpp>
#include "SmppProxy.hpp"
#include "IOContextPool.hpp"

int main()
{
    try
    {
        IOContextPool io_pool(4);
        std::vector<std::string> upstream_hosts = { "127.0.0.1" };

        auto proxy = std::make_shared<SmppProxy>(io_pool, upstream_hosts, 3000);
        proxy->start();

        // Handle Ctrl+C / kill signal
        boost::asio::signal_set signals(io_pool.get_next_io_context(), SIGINT, SIGTERM);
        signals.async_wait([&](const boost::system::error_code&, int signal_number) {
            spdlog::warn("Received signal {}, shutting down gracefully...", signal_number);
            io_pool.stop();
        });

        io_pool.run();
        io_pool.join();

        spdlog::info("Exited io_context loop. Bye!");
    }
    catch (const std::exception& ex)
    {
        spdlog::error("Fatal error: {}", ex.what());
    }
}
