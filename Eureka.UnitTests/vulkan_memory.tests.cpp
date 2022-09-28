#include <catch.hpp>
#include <debugger_trace.hpp>
#include "../Eureka.Vulkan/Instance.hpp"
#include "../Eureka.Vulkan/Device.hpp"
#include "../Eureka.Vulkan/ResourceAllocator.hpp"

namespace vk = eureka::vulkan;

TEST_CASE("pool allocation", "[vulkan]")
{
    constexpr uint64_t CHUNK_SIZE = 1024;
    constexpr uint64_t CHUNK_COUNT = 4;
    constexpr uint64_t POOL_SIZE = CHUNK_SIZE * CHUNK_COUNT;

    auto instance = vk::MakeDefaultInstance();
    auto device = vk::MakeDefaultDevice(instance);
    auto memoryAllocator = std::make_shared<vk::ResourceAllocator>(instance, device);
    auto poolAllocator = std::make_shared<vk::BufferMemoryPool>(
        memoryAllocator, 
        POOL_SIZE,
        VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT
        );
    


    SECTION("full size alloc")
    {
        vk::BufferAllocation alloc1{};
        vk::BufferAllocation alloc2{};
        REQUIRE_NOTHROW(alloc1 = poolAllocator->AllocateBuffer(POOL_SIZE));
        REQUIRE_NOTHROW(poolAllocator->DeallocateBuffer(alloc1));
        REQUIRE_NOTHROW(alloc1 = poolAllocator->AllocateBuffer(POOL_SIZE / 2));
        REQUIRE_NOTHROW(alloc2 = poolAllocator->AllocateBuffer(POOL_SIZE / 2));
        REQUIRE_NOTHROW(poolAllocator->DeallocateBuffer(alloc1));
        REQUIRE_NOTHROW(poolAllocator->DeallocateBuffer(alloc2));
        REQUIRE_NOTHROW(alloc1 = poolAllocator->AllocateBuffer(POOL_SIZE / 2));
        REQUIRE_NOTHROW(alloc2 = poolAllocator->AllocateBuffer(POOL_SIZE / 2));
        REQUIRE_NOTHROW(poolAllocator->DeallocateBuffer(alloc1));
        REQUIRE_NOTHROW(poolAllocator->DeallocateBuffer(alloc2));
    }

    SECTION("> size alloc")
    {
        vk::BufferAllocation alloc1{};
        REQUIRE_THROWS(alloc1 = poolAllocator->AllocateBuffer(POOL_SIZE * 2));
    }





}