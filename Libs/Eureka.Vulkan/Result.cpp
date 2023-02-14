#include "Result.hpp"

namespace eureka::vulkan
{

    void CheckResult(VkResult res)
    {
        if(res != VkResult::VK_SUCCESS)
        {
            throw ResultError(res, "vulkan api error");
        }
    }

} // namespace eureka::vulkan
