
#include <cassert>


#ifndef EUREKA_ASSERT_MACRO

#define EUREKA_ASSERT(cond, str) assert(cond && str)

#endif