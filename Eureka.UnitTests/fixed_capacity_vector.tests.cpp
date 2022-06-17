#include <catch2/catch.hpp>
#include <debugger_trace.hpp>
#include <fixed_capacity_vector.hpp>

namespace eureka
{


}
TEST_CASE("fixed size vector", "[fixed_capacity_vector]")
{
    eureka::fixed_capacity_vector<int> vec1(5);
    eureka::fixed_capacity_vector<int> vec2(5);
}
