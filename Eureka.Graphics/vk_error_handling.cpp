#include "vk_runtime.hpp"
#include <debugger_trace.hpp>

namespace eureka
{
    void CheckVulkan(vk::Result res, const char* stmt, const char* fname, int line)
    {
        if (res != vk::Result::eSuccess)
        {
            auto str = formatted_debugger_line(stmt, fname, line, "vulkan api error");
            debug_output(str);
            vk::throwResultException(res, "vulkan api error");
        }
    }


    void CheckVulkan(vk::Result res)
    {
        if (res != vk::Result::eSuccess)
        {
            vk::throwResultException(res, "vulkan api error");
        }
    }

}
