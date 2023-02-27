#pragma once
#include <spdlog/spdlog.h>
//#include <source_location>
//#include <chrono>
//#include <iostream>
//#include "formatter_specializations.hpp"

//namespace eureka
//{
//    template<std::size_t LINE, std::size_t FILNAME_N>
//    inline consteval auto consteval_append_line_and_date(std::array<char, FILNAME_N> filename)
//    {
//        constexpr char DATE_TIME_FORMAT[] = " | {:%F %T %Z} | ";
//
//        constexpr std::size_t DATE_TIME_FORMAT_N = sizeof(DATE_TIME_FORMAT) - 1; // no null character
//
//        constexpr std::size_t DIGITS_N = constevel_count_digits(LINE);
//        constexpr std::size_t TOTAL_N = FILNAME_N + (1 + DIGITS_N + 1) + DATE_TIME_FORMAT_N; // '(' ')'
//
//        std::array<char, TOTAL_N> result;
//        std::size_t current = 0;
//
//        for (std::size_t i = 0; i < FILNAME_N; ++i)
//        {
//            result[current++] = filename[i];
//        }
//        result[current] = '(';
//        auto line = LINE;
//        for (std::size_t i = DIGITS_N; i > 0; --i)
//        {
//            auto digit = line % 10;
//            line /= 10;
//            result[current + i] = static_cast<char>(static_cast<int>('0') + digit);
//        }
//
//        current += (DIGITS_N + 1);
//        result[current++] = ')';
//
//        for (std::size_t i = 0; i < DATE_TIME_FORMAT_N; ++i)
//        {
//            result[current++] = DATE_TIME_FORMAT[i];
//        }
//
//        return result;
//    }
//
//
//    template<std::size_t N1, std::size_t N2>
//    inline consteval std::array<char, N1 + N2 + 1> consteval_add_arrays_and_newline(std::array<char, N1> a1, std::array<char, N2> a2)
//    {
//        std::array<char, N1 + N2 + 1> result;
//
//        int current = 0;
//
//        for (std::size_t i = 0; i < N1; ++i)
//        {
//            result[current++] = a1[i];
//        }
//        for (std::size_t i = 0; i < N2; ++i)
//        {
//            result[current++] = a2[i];
//        }
//        result[current++] = '\n';
//        return result;
//    }
//    template<std::size_t LINE, std::size_t START, std::size_t N, std::size_t USER_N>
//    inline consteval auto consteval_log_format(const char(&file_str)[N], const char(&user_str)[USER_N])
//    {
//        auto filename = consteval_substring<START>(file_str);
//        auto user_fmt = consteval_substring<0>(user_str);
//        auto file_and_date_log_format = consteval_append_line_and_date<LINE>(filename);
//        auto final_format = consteval_add_arrays_and_newline(file_and_date_log_format, user_fmt);
//        return final_format;
//    }
//
//
//
//    inline void console_log(const std::string& str)
//    {
//        std::cout << str;
//    }
//
//    inline auto local_zoned_time()
//    {
//        return std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::system_clock::now() };
//    }
//
//}
//
//#define CLOG(user_format, ...) eureka::console_log(eureka::fmt_arr(eureka::consteval_log_format<__LINE__, eureka::consteval_file_name_offset(__FILE__)>(__FILE__, user_format), eureka::local_zoned_time(), ##__VA_ARGS__))
//
