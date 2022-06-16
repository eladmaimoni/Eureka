

add_executable(
    bin2h
    "bin2h_main.cpp"
    "bin2h.hpp"
    "bin2h.cpp"    
) 

set_compiler_flags(bin2h)   

target_link_libraries(
    bin2h
    PRIVATE 
)
