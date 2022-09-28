#pragma once
#include <volk.h>
#include <stdexcept>

namespace eureka::vulkan
{
    class ResultError : public std::runtime_error
    {
        VkResult _result;
    public:
        ResultError(VkResult res, const char* msg)
            : std::runtime_error(msg), _result(res)
        {

        }
        ResultError(VkResult res, const std::string msg)
            : std::runtime_error(msg), _result(res)
        {

        }
    };

    //void CheckResult(VkResult res, const char* stmt, const char* fname, int line)
    //{
    //    if (res != vk::Result::eSuccess)
    //    {
    //        auto str = formatted_debugger_line(stmt, fname, line, "vulkan api error");
    //        debug_output(str);
    //        vk::throwResultException(res, "vulkan api error");
    //    }
    //}


    void CheckResult(VkResult res);


}


#ifndef VK_CHECK
//#ifdef NDEBUG
//#define VK_CHECK(stmt) stmt
//#else
#define VK_CHECK(stmt) eureka::vulkan::CheckResult(stmt); 
#endif
//#endif