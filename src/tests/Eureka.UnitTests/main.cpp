
#ifndef CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_RUNNER
#endif
#include <catch.hpp>
#ifdef WIN32
#pragma comment(lib, "ws2_32.lib")
#endif
/*
useful docs & notes

configuring command line arguments
https://docs.microsoft.com/en-us/cpp/build/configure-cmake-debugging-sessions?view=msvc-170
NOTE, to supply command arguments with space bar:
 "args": ["\"FIRST ARGUMENT\"", "\"SECOND ARGUMENT\""]

*/
int main(int argc, char* argv[])
{
    int result = Catch::Session().run(argc, argv);
    return result;
}

