#include "debugger_trace_impl.hpp"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h> // OutputDebugStringA
#endif


namespace eureka
{
    void VSOutputDebugString([[maybe_unused]] const char* str)
    {
#ifdef _WIN32
        OutputDebugStringA(str);
#endif
    }
    std::string formatted_debugger_line(const char* stmt, const char* fname, int line, const char* extra)
    {
        return eureka::format("{}({}): {} : {}\n", fname, line, stmt, extra);
    }
}