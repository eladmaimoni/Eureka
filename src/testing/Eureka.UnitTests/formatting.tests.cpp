#include <catch.hpp>
//#include <debugger_trace.hpp>
#include <format>
#include <formatter_specializations.hpp>
#include <basic_concepts.hpp>


namespace eureka
{



    //template<typename Object>
    //concept enumerable_that_has_vk_to_string = enumerable<Object> && has_vk_to_string<Object>;
    //template<typename Object>
    //concept enumerable_that_has_eureka_to_string = enumerable<Object> && has_eureka_to_string<Object>;
}

namespace std
{
    //template<eureka::enumerable_that_has_vk_to_string T, class CharT>
    //struct formatter<T, CharT>
    //{
    //    template <typename FormatParseContext>
    //    auto parse(FormatParseContext& pc)
    //    {
    //        // parse formatter args like padding, precision if you support it
    //        return pc.end(); // returns the iterator to the last parsed character in the format string, in this case we just swallow everything
    //    }

    //    template<typename FormatContext>
    //    auto format(const T& obj, FormatContext& fc)
    //    {
    //        return std::format_to(fc.out(), "{}", vk::to_string(obj));
    //    }
    //};

    //template<eureka::has_eureka_to_string T, class CharT>
    //struct formatter<T, CharT>
    //{
    //    template <typename FormatParseContext>
    //    auto parse(FormatParseContext& pc)
    //    {
    //        // parse formatter args like padding, precision if you support it
    //        return pc.end(); // returns the iterator to the last parsed character in the format string, in this case we just swallow everything
    //    }

    //    template<typename FormatContext>
    //    auto format(const T& obj, FormatContext& fc)
    //    {
    //        return std::format_to(fc.out(), "{}", eureka::to_string(obj));
    //    }
    //};


}

namespace eureka
{


    struct NeitherStreamableNorFormattable
    {
        int val;
    };
    struct StreamableNotFormattable
    {
        int val;
    };

    std::ostream& operator<<(std::ostream& os, const StreamableNotFormattable& obj)
    {
        return os << "StreamableNotFormattable{" << obj.val << "}";
    }

    static_assert(streamable<StreamableNotFormattable>);
    static_assert(streamable<StreamableNotFormattable&>);
    static_assert(streamable<std::string>);
    static_assert(streamable<std::string>);
    static_assert(streamable<std::string&>);
    static_assert(streamable<const std::string&>);
    static_assert(streamable<int>);
    static_assert(streamable<int&>);
    static_assert(!streamable<std::vector<int>>);
    static_assert(!streamable<std::vector<int>&>);
    static_assert(iterable<std::vector<int>>);
    static_assert(iterable<std::vector<int>&>);
    static_assert(iterable<std::string>);
    //static_assert(!has_vk_to_string<std::string>);
    //static_assert(has_vk_to_string<vk::FormatFeatureFlags2KHR>);
    //static_assert(!has_vk_to_string_not_streamable_or_enum<vk::Result>);
    //static_assert(enumerable_that_has_vk_to_string<vk::Result>);
    


    
}

TEST_CASE("formating string", "[formatting]")
{
    //auto result = std::vformat("{}", std::make_format_args(std::string("hi")));
    auto result = std::format("{}", std::string("hi"));

    CHECK(result == "hi");
}

TEST_CASE("formatting streamable that is not formattable", "[formatting]")
{
    auto result = std::vformat("{}", std::make_format_args(eureka::StreamableNotFormattable{ 123 })); 
    //auto result = std::format("{}", eureka::StreamableNotFormattable{ 123 }); // clang WTF?

    CHECK(result == "StreamableNotFormattable{123}");
}

TEST_CASE("formatting iterable of formattable", "[formatting]")
{
    std::vector<int> vals{ 1,2,3 };
    auto result = std::vformat("{}", std::make_format_args(vals));  // clang WTF?
    //auto result = std::format("{}", vals);

    CHECK(result == "{1,2,3}");
}

TEST_CASE("formatting iterable of streamable that is not formattable", "[formatting]")
{
    std::vector<eureka::StreamableNotFormattable> vals{ {1},{2} };
    auto result = std::vformat("{}", std::make_format_args(vals)); // clang??
    //auto result = std::format("{}", vals);

    CHECK(result == "{StreamableNotFormattable{1},StreamableNotFormattable{2}}");
}


TEST_CASE("formatting eigen matrix", "[formatting]")
{
    auto mat = Eigen::Matrix3d::Identity();
    auto result = std::vformat("{}", std::make_format_args(mat)); // clang??
    //auto result = std::format("{}", mat);
    auto expected = "1 0 0\n0 1 0\n0 0 1";

    CHECK(result == expected);
}