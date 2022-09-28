#pragma once
#include <volk.h>

namespace eureka::vulkan
{
    class Queue
    {
        uint32_t  _family{};
        VkQueue _queue{};
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

