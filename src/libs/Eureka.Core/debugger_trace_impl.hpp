#pragma once
#include <string_view>
#include<array>
#include "formatter_specializations.hpp"

namespace eureka
{
    inline constexpr char LOG_FORMAT[] = "{}({}): ";
    inline constexpr char LOG_FORMAT_LINE_NUM[] = "({}): ";
    inline const std::string LOG_FORMAT_S(LOG_FORMAT);

    void VSOutputDebugString(const char* str);

    std::true_type constexpr return_true()
    {
        return std::true_type();
    }


    static_assert(constevel_count_digits(1234) == 4);

    template <
        std::size_t LINE_NUM, // __LINE__
        std::size_t LINE_STR_N, // __FILE__ string literal length
        std::size_t USER_STR_N // user string literal length 
    >
        inline constexpr auto append_debugger_format_internal(
            const char(&line_str)[LINE_STR_N],
            const char(&user_str)[USER_STR_N],
            std::true_type // means user_str is a string literal
        )
    {
        // compile time formatting of log message
        constexpr std::size_t LINE_DIGITS_N = constevel_count_digits(LINE_NUM);

        constexpr std::size_t TOTAL = (LINE_STR_N - 1) + 1 + (LINE_DIGITS_N)+3 + (USER_STR_N - 1) + 2;
        std::array<char, TOTAL> formatted_str{};

        auto current = 0;
        for (auto i = 0u; i < LINE_STR_N - 1; ++i)
        {
            formatted_str[current++] = line_str[i];
        }

        // parentheses & digits
        formatted_str[current] = '(';
        auto line = LINE_NUM;
        for (auto i = LINE_DIGITS_N; i > 0; --i)
        {
            auto digit = line % 10;
            line /= 10;
            formatted_str[current + i] = static_cast<char>(static_cast<int>('0') + digit);
        }

        current += (LINE_DIGITS_N + 1);

        formatted_str[current++] = ')';
        formatted_str[current++] = ':';
        formatted_str[current++] = ' ';


        for (auto i = 0u; i < (USER_STR_N - 1); ++i)
        {
            formatted_str[current++] = user_str[i];
        }

        formatted_str[current++] = '\n';
        formatted_str[TOTAL - 1] = 0;

        return formatted_str;
    }

    template <
        std::size_t LINE_NUM,// __LINE__
        std::size_t LINE_STR_N, // __FILE__ string literal length
        std::size_t USER_STR_N, // user string literal length 
        //typename LineStrT, // __FILE__
        //typename UserStrT, // either string literal or const char*
        typename ... Args
    >
        inline constexpr auto append_debugger_format_internal(
            const char(&line_str)[LINE_STR_N],
            const char(&user_str)[USER_STR_N],
            //const LineStrT& line_str,
            //UserStrT&& user_str,
            std::true_type, // means user_str is a string literal
            Args&& ... args
        )
    {
        const auto formatted_arr = append_debugger_format_internal<LINE_NUM>(line_str, user_str, return_true()); // compiler bug when making this variable constexpr?
        std::string_view formatted_string{ formatted_arr.data(), formatted_arr.size() };
        return std::vformat(formatted_string, std::make_format_args(std::forward<Args>(args) ...));
    }

    template <
        std::size_t LINE_NUM, // __LINE__
        std::size_t LINE_STR_N, // __FILE__ string literal length
        typename ... Args
    >
        inline constexpr auto append_debugger_format_internal(
            const char(&line_str)[LINE_STR_N],
            const char* user_str, // user format that may be concatenated with parameters
            std::false_type,
            Args&& ... args
        )
    {
        // log format and user str can't be formatted at compile time because user_str is not a string literal 
        // still could probably be optimized
        return std::vformat(LOG_FORMAT_S + user_str + '\n', line_str, LINE_STR_N, std::make_format_args(std::forward<Args>(args) ...));
    }


    template<
        std::size_t LINE_NUM,// __LINE__
        typename LineStrT, // __FILE__
        typename UserStrT, // either string literal or const char*
        typename = std::enable_if_t<std::is_convertible_v<UserStrT, const char*>>,
        typename ... Args
    >
        inline constexpr auto append_debugger_format(const LineStrT& line_str, UserStrT&& user_str, Args&& ... args)
    {
        // may return std::array when string literals

        // https://stackoverflow.com/questions/32513517/c-overload-static-const-string-vs-char-array
        return append_debugger_format_internal<LINE_NUM>(
            line_str,
            user_str,
            std::is_array<std::remove_reference_t<UserStrT>>(),
            std::forward<Args>(args) ...
            );
    }

    inline void debug_output(const std::string& str)
    {
        VSOutputDebugString(str.c_str());
    }

    template <std::size_t N, typename ... Args>
    inline void debug_output([[maybe_unused]] const std::array<char, N>& arr, [[maybe_unused]] Args&& ... args)
    {
        std::string_view str{ arr.data(), arr.size() };
        VSOutputDebugString(std::vformat(str, std::make_format_args(std::forward<Args>(args) ...)).c_str());
    }


}
