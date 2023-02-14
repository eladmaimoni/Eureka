#include <boost/container/small_vector.hpp>
#include <concurrencpp/concurrencpp.h>
#include <compiler.hpp>
#include <containers_aliases.hpp>
#include <macros.hpp>
#include <function2/function2.hpp>
#include <sigslot/signal.hpp>
EUREKA_MSVC_WARNING_PUSH_DISABLE(5054 4127)
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/Dense>
EUREKA_MSVC_WARNING_POP

namespace fu2 {}

using namespace std::chrono_literals;

namespace fu = fu2;
namespace cuncur = concurrencpp;

namespace eureka
{
    template<typename T> using promise_t = concurrencpp::result_promise<T>;
    template<typename T> using future_t = concurrencpp::result<T>;

    template <typename... Signatures> 
    using function_t = fu2::function<Signatures ...>;
 
    
}