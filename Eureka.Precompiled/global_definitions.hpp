#include <boost/container/small_vector.hpp>
#include <concurrencpp/concurrencpp.h>

namespace vk::raii {}
namespace fu2 {}
namespace concurrencpp {};

using namespace std::chrono_literals;
namespace vkr = vk::raii;
namespace fu = fu2;
namespace cuncur = concurrencpp;

namespace eureka
{
    template<typename T> using svec5 = boost::container::small_vector<T, 5>;
    template<typename T> using svec10 = boost::container::small_vector<T, 10>;
    template<typename T> using svec15 = boost::container::small_vector<T, 15>;
    template<typename T> using svec20 = boost::container::small_vector<T, 20>;
}