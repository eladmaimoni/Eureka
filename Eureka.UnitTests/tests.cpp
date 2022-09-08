#include <catch2/catch.hpp>
#include <random>
#include <debugger_trace.hpp>
#include <boost/container/small_vector.hpp>

TEST_CASE("small vector", "[vulkan]")
{
    std::vector<int> outer_vec;
    boost::container::small_vector<int, 10> outer_svec;
    BENCHMARK("small vector on heap")
    {
        std::vector<int> vec(10);

        for (auto i = 0; i < 20; ++i)
        {
            vec.emplace_back(i);
        }

        outer_vec = std::move(vec);
    };


    BENCHMARK("small vector")
    {
        boost::container::small_vector<int, 20> svec;
        for (auto i = 0; i < 20; ++i)
        {
            svec.emplace_back(i);
        }

        outer_svec = std::move(svec);
    };
}

