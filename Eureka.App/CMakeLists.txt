
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

if (WIN32)
    set(SUBSYSTEM_LINKER_OPTIONS "$<IF:$<CONFIG:Debug>,/SUBSYSTEM:CONSOLE,/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup>")
else()
    set(SUBSYSTEM_LINKER_OPTIONS "$<IF:$<NOT:$<CONFIG:Debug>>,-mwindows,>")
endif()

target_link_options(Eureka.App PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})

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
    Eureka.Serialization
    Eureka.Interfaces
    Eureka.Graphics
    Eureka.Vulkan
    Eureka.Windowing
    Eureka.Shaders
    Eureka.RPC
    Eureka.RemoteProto
    Eureka.RemoteClient
    concurrencpp::concurrencpp
)