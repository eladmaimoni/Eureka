#include <catch2/catch.hpp>
#include <random>
#include <debugger_trace.hpp>
#include <AssetLoading.hpp>


TEST_CASE("model", "[vulkan]")
{
    concurrencpp::runtime_options options{};
    concurrencpp::runtime runtime;

    //runtime.
    auto result = runtime.thread_pool_executor()->submit([] {
        DEBUGGER_TRACE("hi");

        });

    auto result_bk = runtime.thread_pool_executor()->submit([] {
        DEBUGGER_TRACE("hi");

        });

    result.get();


    std::filesystem::path modelPath = "C:/Projects/Samples/Vulkan/data/models/treasure_smooth.gltf";

    auto model = eureka::LoadModel(modelPath);

    CHECK(true);

}

