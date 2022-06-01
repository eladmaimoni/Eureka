


add_executable(
    Eureka.App
    "EurekaMain.cpp"
    "App.hpp"
    "App.cpp"
    "UIView.hpp"
    "UIView.cpp"
    "IOCContainer.hpp"
    "IOCContainer.cpp"
) 

set_compiler_flags(Eureka.App)  


target_shared_precompiled_headers(
    Eureka.App 
    Eureka.Precompiled
)

target_link_libraries(
    Eureka.App
    PRIVATE 
    Eureka.Precompiled
    Eureka.Core
    Eureka.Graphics
    Eureka.Shaders
)