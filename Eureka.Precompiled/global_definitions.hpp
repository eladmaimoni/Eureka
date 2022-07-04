#include <boost/container/small_vector.hpp>
#include <concurrencpp/concurrencpp.h>
#include <compiler.hpp>
#include <macros.hpp>
#include <Profiling.hpp>
#include <function2/function2.hpp>
#include <sigslot/signal.hpp>
EUREKA_MSVC_WARNING_PUSH_DISABLE(5054 4127)
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/Dense>
EUREKA_MSVC_WARNING_POP

namespace vk::raii {}
namespace fu2 {}


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


    template<typename T> using promise_t = concurrencpp::result_promise<T>;
    template<typename T> using future_t = concurrencpp::result<T>;
  
    template<typename T> using dynamic_span = std::span<T, std::dynamic_extent>; 
    template<typename T> using dynamic_cspan = std::span<const T, std::dynamic_extent>;

    template <typename... Signatures> 
    using function_t = fu2::function<Signatures ...>;
 
    
}