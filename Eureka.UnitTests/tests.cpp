#include <catch2/catch.hpp>
#include <random>
#include <debugger_trace.hpp>
#include <AssetLoading.hpp>


TEST_CASE("model", "[vulkan]")
{
    std::filesystem::path modelPath = "C:/Projects/Samples/Vulkan/data/models/treasure_smooth.gltf";

    auto model = eureka::LoadModel(modelPath);

    CHECK(true);

}

