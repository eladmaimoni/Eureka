#pragma once

/*
https://stackoverflow.com/questions/59909102/stdformat-of-user-defined-types
https://madridccppug.github.io/posts/stdformat/
*/

#include "basic_concepts.hpp"
#include <sstream>

namespace eureka
{
    template<typename Iterable>
    concept iterable_of_formattable = (
        !streamable<std::decay_t<Iterable>> && 
        iterable<std::decay_t<Iterable>> &&
        formattable<iterable_value_t<Iterable>>
        );
}


namespace std
{
    template<eureka::streamable T, class CharT>
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
            std::ostringstream out;
            out << obj;
            return std::format_to(fc.out(), "{}", out.str());
        }
    };



    template<eureka::iterable_of_formattable T, class CharT>
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
            std::format_to(fc.out(), "{}", '{');

            auto itr = std::begin(obj);
            auto end = std::end(obj);

            if (std::distance(itr, end) > 0)
            {
                std::format_to(fc.out(), "{}", *(itr++));

                for (; itr != end; ++itr)
                {
                    std::format_to(fc.out(), ",{}", *itr);
                }
            }
            return std::format_to(fc.out(), "{}", '}');
        }
    };


}