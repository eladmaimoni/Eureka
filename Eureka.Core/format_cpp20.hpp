#pragma once
#if (__cpp_lib_format && __cpp_lib_concepts)
#include <concepts>
#include <format>
#include <sstream>
#include <type_traits>

namespace eureka
{
    template <typename Iterable>
    struct iterable_value
    {
        using type = std::decay_t<decltype(*std::begin(std::declval<Iterable>()))>;
    };
    
    template <typename Iterable>
    using iterable_value_t = iterable_value<Iterable>::type;

    //
    // has_eureka_to_string : a type that has eureka::to_string overloaded
    //
    template<typename Object>
    concept has_eureka_to_string = requires(const Object & obj)
    {
        to_string(obj);
    };

    //
    // streamable : a type that has operator << overloaded
    //
    template<typename Object>
    concept streamable = requires(std::ostream& os, const Object& obj)
    {
        os << obj; 
    };

    //
    // iterable : a type that can be used in a ranged-based for loop 
    //
    template<typename Iterable>
    concept iterable = requires(const Iterable & obj)
    {
        std::begin(obj);
        std::end(obj);

        // not sure this is the best way
        //std::for_each(std::begin(obj), std::end(obj), [](const auto&) {});
    };

    //
    // iterable_of_streamable : 
    // - a type that can be used in a ranged-based for loop 
    // - contains streamable values
    // - is not streamable itself (like std::string)
    // 
    template<typename Iterable>
    concept iterable_of_streamable = (
        iterable<std::decay_t<Iterable>> && 
        !streamable<std::decay_t<Iterable>> && 
        streamable<iterable_value_t<Iterable>>
        );

    template<typename Object>
    concept streamable_not_fundmental_or_string = (
        streamable<Object> &&
        !std::is_same_v<std::decay_t<Object>, std::string> &&
        !std::is_fundamental_v<std::decay_t<Object>>
        );

    template<typename Iterable>
    concept iterable_of_to_stringable = (
        iterable<std::decay_t<Iterable>> && 
        !streamable<std::decay_t<Iterable>> && 
        !streamable<iterable_value_t<Iterable>> &&
        has_eureka_to_string<iterable_value_t<Iterable>>
        );


    std::string to_format_acceptable(has_eureka_to_string auto&& obj)
    {
        return to_string(obj);
    }

    std::string to_format_acceptable(streamable_not_fundmental_or_string auto&& obj)
    {
        std::ostringstream out;
        out << obj;
        return out.str();
    }

    std::string to_format_acceptable(iterable_of_streamable auto&& obj)
    {
        std::ostringstream out;
        out << '{';
        for (const auto& v : obj)
        {
            out << v << ", ";
        }
        out << '}';
        return out.str();
    }


    std::string to_format_acceptable(iterable_of_to_stringable auto&& obj)
    {
        //static_assert<decltype(obj)> 
        std::ostringstream out;
        out << '{';
        for (const auto& v : obj)
        {
            out << to_string(v) << ", ";
        }
        out << '}';
        return out.str();
    }


    template<typename T>
    auto to_format_acceptable(T&& obj) -> decltype(std::forward<T>(obj))
    {
        //using type = std::decay_t<T>;
        //return type{};
        return std::forward<T>(obj);
    }
}

#endif


