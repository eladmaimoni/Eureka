#include "App.hpp"
#include <debugger_trace.hpp>


using namespace eureka;

int main()
{
    try
    {
        DEBUGGER_TRACE("createing app");
        App app;
  
        app.Run();


    }
    catch (const std::exception& err)
    {
        DEBUGGER_TRACE("{}", err.what());
    }

    return 0;
}