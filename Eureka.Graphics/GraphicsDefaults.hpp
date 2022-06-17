#pragma once


namespace eureka
{




    using ManualExecutor = concurrencpp::manual_executor;
    using OneShotCopySubmitExecutor = std::shared_ptr<concurrencpp::manual_executor>;
    using IOExecutor = std::shared_ptr<concurrencpp::thread_pool_executor>;
    using PoolExecutor = std::shared_ptr<concurrencpp::thread_pool_executor>;
    using SecondaryCommandsExecutor = std::shared_ptr<concurrencpp::worker_thread_executor>;

    template<typename T>
    using result_t = concurrencpp::result<T>;

    inline constexpr uint64_t STAGE_ZONE_SIZE = 1024 * 1024 * 400; // 20MB
}