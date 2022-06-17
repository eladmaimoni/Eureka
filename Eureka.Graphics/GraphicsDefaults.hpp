#pragma once


namespace eureka
{

    inline constexpr vk::Format DEFAULT_DEPTH_BUFFER_FORMAT = vk::Format::eD24UnormS8Uint;

    using CopySubmitExecutor = std::shared_ptr<concurrencpp::manual_executor>;
    using IOExecutor = std::shared_ptr<concurrencpp::thread_pool_executor>;
    using PoolExecutor = std::shared_ptr<concurrencpp::thread_pool_executor>;
    using SecondaryCommandsExecutor = std::shared_ptr<concurrencpp::worker_thread_executor>;

    template<typename T>
    using result_t = concurrencpp::result<T>;
}