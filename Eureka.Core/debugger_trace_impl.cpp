#include "debugger_trace_impl.hpp"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h> // OutputDebugStringA
#endif
#include <iostream>
#include <vector>
namespace eureka
{   
    struct StreamableItem
    {
        int jjj;
    };

    inline std::ostream& operator<<(std::ostream& os, const StreamableItem& item)
    {
        return os << item.jjj;
    }
    void PrintTest(const streamable auto& obj)
    {
        std::cout << obj;
    }

    void PrintTestContainer(const iterable_of_streamable auto& iterable)
    {
        for (auto& obj : iterable)
        {
            std::cout << obj;
        }      
    }



    static_assert(streamable<StreamableItem>);
    static_assert(streamable<StreamableItem&>);
    static_assert(streamable<std::string>);
    static_assert(streamable<std::string>);
    static_assert(streamable<std::string&>);
    static_assert(streamable<const std::string&>);
    static_assert(streamable<int>);
    static_assert(streamable<int&>);
    static_assert(streamable_not_fundmental_or_string<StreamableItem>);
    static_assert(streamable_not_fundmental_or_string<StreamableItem&>);
    static_assert(!streamable_not_fundmental_or_string<std::string>);
    static_assert(!streamable_not_fundmental_or_string<std::string&>);
    static_assert(!streamable_not_fundmental_or_string<int>);
    static_assert(!streamable_not_fundmental_or_string<int&>);

    static_assert(!streamable<std::vector<int>>);
    static_assert(!streamable<std::vector<int>&>);
    static_assert(iterable<std::vector<int>>);
    static_assert(iterable<std::vector<int>&>);
    static_assert(iterable<std::string>);
    static_assert(iterable_of_streamable<std::vector<int>>);
    static_assert(iterable_of_streamable<std::vector<int>&>);
    static_assert(iterable_of_streamable<std::vector<std::string>>);
    static_assert(iterable_of_streamable<std::vector<std::string>&>);
    static_assert(!iterable_of_streamable<std::string>);
    static_assert(!iterable_of_streamable<std::string&>);
    static_assert(!has_eureka_to_string<std::string>);
    static_assert(!has_eureka_to_string<int>);
    //static_assert(!has_eureka_to_string<std::vector<int>>);


    //static_assert(iterable_of_streamable_not_string < std::vector<int>::value);

    void TestPrintTest()
    {

        //std::vector<int> vec_int;
        //std::cout << vec_int;
    //    std::ostringstream os;
    //    os << "ok";
    //    std::string str("dafhgfdhgfshj");
    //    PrintTest(str);

    //    
    //    StreamableItem item{ 567 };
    //    std::vector<std::string> vec1;
    //    std::vector<int> vec2;
    //    //std::vector<std::string>::value_type
    //    std::vector<std::vector<int>> vec3;
    //    int jjj = 8;
    //    int ddd = 8.0;

    //    auto f1 = as_format_acceptable2(vec1);
    //    auto f2 = as_format_acceptable2(item);
    //    auto f3 = as_format_acceptable2(str);
    //    auto f4 = as_format_acceptable2(jjj);
    //    auto f5 = as_format_acceptable2(ddd);

    //    //auto f3 = as_format_acceptable2(vec2);



    //    //PrintTestContainer(str);

    //    //PrintTestContainer(vec2);

    }

    void VSOutputDebugString([[maybe_unused]] const char* str)
    {
#ifdef _WIN32
        OutputDebugStringA(str);
#endif
    }
    std::string formatted_debugger_line(const char* stmt, const char* fname, int line, const char* extra)
    {
        return eureka::format("{}({}): {} : {}\n", fname, line, stmt, extra);
    }
}