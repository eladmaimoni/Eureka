#pragma once
#include <volk.h>

namespace eureka::vulkan
{
    class Queue
    {
        VkQueue   _queue{};
        uint32_t  _family{};
    public:
        Queue() = default;
        Queue(VkQueue queue, uint32_t family);
        
        uint32_t Family() const;
        VkQueue Get() const { return _queue; }
        void WaitIdle();

        [[nodiscard]] VkResult Present(const VkPresentInfoKHR& presentInfo);
        void Submit(const VkSubmitInfo& submitInfo, VkFence fence);
    };
}

