
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
    ImGuiApp
    ${app}
) 

set_target_properties(ImGuiApp PROPERTIES FOLDER "Examples")

if (WIN32)
    set(SUBSYSTEM_LINKER_OPTIONS "$<IF:$<CONFIG:Debug>,/SUBSYSTEM:CONSOLE,/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup>")
else()
    set(SUBSYSTEM_LINKER_OPTIONS "$<IF:$<NOT:$<CONFIG:Debug>>,-mwindows,>")
endif()

target_link_options(ImGuiApp PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})

set_compiler_flags(ImGuiApp)  

target_precompile_headers(
    ImGuiApp 
    REUSE_FROM
    Eureka.Precompiled
)

target_link_libraries(
    ImGuiApp
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