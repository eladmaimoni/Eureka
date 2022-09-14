
set_source_group(app 
    "EurekaMain.cpp"
    "App.hpp"
    "App.cpp"
    "UIView.hpp"
    "UIView.cpp"
    "IOCContainer.hpp"
    "IOCContainer.cpp"
) 
  
add_executable(
    Eureka.App
    ${app}
) 

set_compiler_flags(Eureka.App)  


target_precompile_headers(
    Eureka.App 
    REUSE_FROM
    Eureka.Precompiled
)

target_link_libraries(
    Eureka.App
    PRIVATE 
    Eureka.Precompiled
    Eureka.Core
    Eureka.Types
    Eureka.AssetLoading
    Eureka.Graphics
    Eureka.Shaders
    concurrencpp::concurrencpp
)