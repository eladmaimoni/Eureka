#include "debugger_trace_impl.hpp"


///
/// DEBUGGER_TRACE macro
/// sends formatted string to the debugger output
/// you can click on the debugger output to jump straight to the relevant line in the source code
/// usage:
/// DEBUGGER_TRACE("my formatted debugging string {}, {}", 123, "456");
/// 
#define DEBUGGER_TRACE(user_format, ...) eureka::debug_output(eureka::append_debugger_format<__LINE__>(__FILE__, user_format, ##__VA_ARGS__))

namespace eureka
{
    
    
    /// 
    /// creates a formatted debugger line
    /// can be used together with eureka::debug_output() to generate a clickable debugger trace
    /// applicable in cases where we wish to use a different __LINE__ and __FILE__ (not the current line)
    /// 
    std::string formatted_debugger_line(const char* stmt, const char* fname, int line, const char* extra);
}
