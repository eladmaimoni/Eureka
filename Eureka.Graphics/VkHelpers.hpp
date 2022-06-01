#pragma once
#include <debugger_trace.hpp>

namespace eureka
{
    //
    // has_eureka_to_string : a type that has eureka::to_string overloaded
    //
    template<typename Object>
    concept has_vk_to_string = requires(const Object & obj)
    {
        vk::to_string(obj);
    };
}

namespace std
{
    template<eureka::has_vk_to_string T, class CharT>
    struct formatter<T, CharT>
    {
        template <typename FormatParseContext>
        auto parse(FormatParseContext& pc)
        {
            // parse formatter args like padding, precision if you support it
            return pc.end(); // returns the iterator to the last parsed character in the format string, in this case we just swallow everything
        }

        template<typename FormatContext>
        auto format(const T& obj, FormatContext& fc)
        {
       
            return std::format_to(fc.out(), "{}", vk::to_string(obj));
        }
    };
}