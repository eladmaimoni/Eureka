

add_executable(
    bin2h
    "bin2h_main.cpp"
    "bin2h.hpp"
    "bin2h.cpp"    
) 

target_link_libraries(
    bin2h
    PRIVATE
    eureka_strict_compiler_flags
)
