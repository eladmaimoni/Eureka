#pragma once
#include <span>
#include <boost/container/small_vector.hpp>

namespace eureka
{
    template<typename T> using svec2 = boost::container::small_vector<T, 2>;
    template<typename T> using svec3 = boost::container::small_vector<T, 3>;
    template<typename T> using svec5 = boost::container::small_vector<T, 5>;
    template<typename T> using svec10 = boost::container::small_vector<T, 10>;
    template<typename T> using svec15 = boost::container::small_vector<T, 15>;
    template<typename T> using svec20 = boost::container::small_vector<T, 20>;


    template<typename T> using dynamic_span = std::span<T, std::dynamic_extent>;
    template<typename T> using dynamic_cspan = std::span<const T, std::dynamic_extent>;
    template<typename T> using dspan = std::span<T, std::dynamic_extent>;
    template<typename T> using dcspan = std::span<const T, std::dynamic_extent>;
}

