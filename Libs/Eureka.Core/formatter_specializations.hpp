#pragma once

/*
https://stackoverflow.com/questions/59909102/stdformat-of-user-defined-types
https://madridccppug.github.io/posts/stdformat/
*/

#include "basic_concepts.hpp"
#include <sstream>

namespace eureka
{
    template <std::size_t N, typename ... Args>
    inline std::string fmt_arr(const std::array<char, N>& arr, Args&& ... args)
    {
        std::string_view str{ arr.data(), arr.size() };

        return std::vformat(str, std::make_format_args(std::forward<Args>(args) ...));
    }

    std::size_t consteval constevel_count_digits(std::integral auto n)
    {
        // counts the decimal digits in a number
        int count = 0;
        while (n != 0)
        {
            n = n / 10;
            ++count;
        }
        return count;
    }

    template <size_t S>
    inline consteval size_t consteval_file_name_offset(const char(&str)[S], size_t i = S - 1)
    {
        // go from end to start until we meet '/' or '\'
        return (str[i] == '/' || str[i] == '\\') ? i + 1 : (i > 0 ? consteval_file_name_offset(str, i - 1) : 0);
    }

    template <typename T>
    inline consteval size_t consteval_file_name_offset(T(&str)[1])
    {
        (void)str;
        return 0;
    }

    template<std::size_t START, std::size_t N>
    consteval std::array<char, N - START - 1> consteval_substring(const char(&file_str)[N])
    {
        std::array<char, N - START - 1> result;
        std::size_t current = 0;
        for (std::size_t i = START; i < N - 1; ++i) // -1 for no null terminator
        {
            result[current++] = file_str[i];
        }
        return result;
    }

    template<typename T, std::size_t N1, std::size_t N2>
    consteval std::array<T, N1 + N2> consteval_add_arrays(std::array<T, N1> a1, std::array<T, N2> a2)
    {
        std::array<T, N1 + N2> result;

        int current = 0;

        for (std::size_t i = 0; i < N1; ++i)
        {
            result[current++] = a1[i];
        }
        for (std::size_t i = 0; i < N2; ++i)
        {
            result[current++] = a2[i];
        }

        return result;
    }
}

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
    template<eureka::streamable_not_enumerable T, class CharT>
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

    template<eureka::has_eureka_to_string T, class CharT>
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
            return std::format_to(fc.out(), "{}", eureka::to_string(obj));
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