


#if defined(_MSC_VER) && _MSC_VER >= 1400
#define EUREKA_MSVC_WARNING_PUSH      __pragma(warning( push ))
#define EUREKA_MSVC_WARNING_DISABLE(...) __pragma(warning(disable:##__VA_ARGS__))
#define EUREKA_MSVC_WARNING_PUSH_DISABLE(...) \
EUREKA_MSVC_WARNING_PUSH \
EUREKA_MSVC_WARNING_DISABLE(##__VA_ARGS__)
#define EUREKA_MSVC_WARNING_SUPPRESS(...) __pragma(warning(suppress:##__VA_ARGS__))
#define EUREKA_MSVC_WARNING_POP __pragma(warning(pop))
#else
#define EUREKA_MSVC_WARNING_PUSH
#define EUREKA_MSVC_WARNING_DISABLE(...)
#define EUREKA_MSVC_WARNING_SUPPRESS(...)
#define EUREKA_MSVC_WARNING_PUSH_DISABLE(...)
#define EUREKA_MSVC_WARNING_POP
#endif