#include <concurrencpp/concurrencpp.h>

namespace eureka
{
    template<typename T> using promise_t = concurrencpp::result_promise<T>;
    template<typename T> using future_t = concurrencpp::result<T>;
}