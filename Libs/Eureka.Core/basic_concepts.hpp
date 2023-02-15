#pragma once
#include <concepts>
#include <type_traits>
#include <ostream>
#include <format>

namespace eureka
{
    struct dummy_to_string {};
    inline std::string to_string(const dummy_to_string&)
    {
        return "";
    }

    template <class T, template <class...> class Template>
    struct is_specialization : std::false_type {};

    template <template <class...> class Template, class... Args>
    struct is_specialization<Template<Args...>, Template> : std::true_type {};



    template <typename Iterable>
    struct iterable_value
    {
        using type = std::decay_t<decltype(*std::begin(std::declval<Iterable>()))>;
    };

    template <typename Iterable>
    using iterable_value_t = typename iterable_value<Iterable>::type;

    template<typename T>
    concept formattable = requires (T & v, std::format_context ctx) 
    {
        std::formatter<std::remove_cvref_t<T>>().format(v, ctx);
    };

    template<typename T>
    concept enumerable = std::is_enum_v<T>;

    template<typename T>
    concept pointer = std::is_pointer_v<T>;
    //
    // has_eureka_to_string : a type that has eureka::to_string overloaded
    //
    template<typename Object>
    concept has_eureka_to_string = requires(const Object & obj)
    {
        eureka::to_string(obj);
    };

    //
    // streamable : a type that has operator << overloaded
    //
    template<typename Object>
    concept streamable = requires(std::ostream & os, const Object & obj)
    {
        os << obj;
    };


    template<typename Object>
    concept streamable_not_enumerable = streamable<Object> && !enumerable<Object>;


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



}