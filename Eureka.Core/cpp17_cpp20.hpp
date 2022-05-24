#pragma once

#ifdef __cpp_consteval 
#define CONSTEVAL consteval
#else
#define CONSTEVAL constexpr
static_assert(false);
#endif