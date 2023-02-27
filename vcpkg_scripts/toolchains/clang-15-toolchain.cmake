

set(CMAKE_C_COMPILER "/usr/bin/clang-15")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++-15")
set(CMAKE_CXX_FLAGS "-fexperimental-library -stdlib=libc++")

message("clang toolchain CMAKE_C_COMPILER = ${CMAKE_C_COMPILER}")
message("clang toolchain CMAKE_CXX_COMPILER = ${CMAKE_CXX_COMPILER}")
