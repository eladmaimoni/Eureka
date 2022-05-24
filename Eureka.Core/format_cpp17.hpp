#pragma once
#include "cpp17_cpp20.hpp"
#include <fmt/format.h>

namespace eureka
{
    namespace format_ns = fmt;

    template<class ...Ts>
    struct voider
    {
        using type = void;
    };

    template<class T, class = void>
    struct has_to_string : std::false_type {};

    template<class T>
    struct has_to_string<T, typename voider<decltype(std::declval<T>().to_string())>::type> : std::true_type {};

    template<class T, class = void>
    struct is_ostream_overload_available : std::false_type {};

    template<class T>
    struct is_ostream_overload_available<T, typename voider<decltype(std::declval<std::ostream&>() << std::declval<T const&>())>::type> : std::true_type {};

    template <
        typename Printable,
        std::enable_if_t<is_ostream_overload_available<Printable>::value, bool> = true
    >
        std::string as_format_acceptable(Printable&& printable)
    {
        std::ostringstream out;
        out << printable;
        return out.str();
    }

    template <
        typename Printable,
        std::enable_if_t<has_to_string<Printable>::value, bool> = true
    >
        std::string as_format_acceptable(Printable&& printable)
    {
        return printable.to_string();
    }

    template <
        typename Printable,
        typename... Ignored
    >
        auto as_format_acceptable(Printable&& printable, Ignored const&...) -> decltype(std::forward<Printable>(printable))
    {
        static_assert(sizeof...(Ignored) == 0, "only one parameter allowed");
        return std::forward<Printable>(printable);
    }
}