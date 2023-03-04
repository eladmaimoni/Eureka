#include "Commands.hpp"
#include <move.hpp>

namespace eureka::vulkan
{
    LinearCommandBufferHandle::LinearCommandBufferHandle(VkCommandBuffer commandBuffer) : _commandBuffer(commandBuffer)
    {

    }

    void LinearCommandBufferHandle::Begin()
    {
        VkCommandBufferBeginInfo beginInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
        };
        VK_CHECK(vkBeginCommandBuffer(_commandBuffer, &beginInfo));
    }

    void LinearCommandBufferHandle::End()
    {
        VK_CHECK(vkEndCommandBuffer(_commandBuffer));
    }

    LinearCommandPool::LinearCommandPool(std::shared_ptr<Device> device, uint32_t queueFamilyIndex) :
        _device(std::move(device))
    {
        VkCommandPoolCreateInfo createInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = 0,
            .queueFamilyIndex = queueFamilyIndex
        };

        _pool = _device->CreateCommandPool(createInfo);
    }

    LinearCommandPool::LinearCommandPool(LinearCommandPool&& that) noexcept :
        _device(std::move(that._device)),
        _pool(steal(that._pool))
    {

    }
    
    LinearCommandPool& LinearCommandPool::operator=(LinearCommandPool&& rhs) noexcept
    {
        _device = std::move(rhs._device);
        _pool = steal(rhs._pool);
        return *this;
    }

    LinearCommandPool::~LinearCommandPool()
    {
        if (nullptr != _pool)
        {
            _device->DestroyCommandPool(_pool);
        }
    }

    void LinearCommandPool::Reset()
    {
        _device->ResetCommandPool(_pool);
    }

    LinearCommandBufferHandle LinearCommandPool::AllocatePrimaryCommandBuffer() const
    {
        return LinearCommandBufferHandle(_device->AllocatePrimaryCommandBuffer(_pool));
    }



}