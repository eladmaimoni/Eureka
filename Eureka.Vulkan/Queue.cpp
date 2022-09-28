#include "Queue.hpp"
#include "Result.hpp"

namespace eureka::vulkan
{
    Queue::Queue(VkQueue queue, uint32_t family) : _queue(queue), _family(family)
    {

    }

    VkResult Queue::Present(const VkPresentInfoKHR& presentInfo)
    {
        return vkQueuePresentKHR(_queue, &presentInfo);
    }

    void Queue::Submit(const VkSubmitInfo& submitInfo, VkFence fence)
    {
        VK_CHECK(vkQueueSubmit(_queue, 1u, &submitInfo, fence));
    }

    void Queue::WaitIdle()
    {
        VK_CHECK(vkQueueWaitIdle(_queue));
    }

    uint32_t Queue::Family() const
    {
        return _family;
    }

}

