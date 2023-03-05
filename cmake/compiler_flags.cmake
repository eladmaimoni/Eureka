#set(EUREKA_NOT_DEBUG "$<OR:$<CONFIG:Release>, $<CONFIG:RelWithDebInfo>>")
set(EUREKA_NOT_DEBUG "$<NOT:$<CONFIG:Debug>>")
set(EUREKA_COMPILER_IS_GCC_LIKE "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(EUREKA_COMPILER_IS_MSVC "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
set(EUREKA_GCC_LIKE_STRICT_FLAGS "-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused")
set(EUREKA_MSVC_STRICT_FLAGS "/W4") # (/EHsc) is somehow not set on Ninja
set(EUREKA_WARNING_AS_ERROR_FLAG "$<IF:${EUREKA_COMPILER_IS_MSVC},/WX,-Werror>")
set(EUREKA_TREAT_WARNING_AS_ERROR "$<${EUREKA_NOT_DEBUG}:${EUREKA_WARNING_AS_ERROR_FLAG}>")


add_library(eureka_strict_compiler_flags INTERFACE)


target_compile_options(
  eureka_strict_compiler_flags 
  INTERFACE
  "$<${EUREKA_COMPILER_IS_GCC_LIKE}:${EUREKA_GCC_LIKE_STRICT_FLAGS}>"
  "$<${EUREKA_COMPILER_IS_MSVC}:${EUREKA_MSVC_STRICT_FLAGS}>"
  "${EUREKA_TREAT_WARNING_AS_ERROR}"
  $<$<CXX_COMPILER_ID:Clang>:-fexperimental-library -stdlib=libc++> # https://discourse.llvm.org/t/linux-what-is-the-status-of-libc-in-llvm-15-apt-packages-ranges-format/65348/4
)

target_link_options(
    eureka_strict_compiler_flags
    INTERFACE
    $<$<CXX_COMPILER_ID:Clang>: -fexperimental-library -stdlib=libc++>
)

target_compile_features(
    eureka_strict_compiler_flags
    INTERFACE
    cxx_std_23
)


function(set_no_warnings_compiler_flags target_name)
    target_compile_features(
        ${target_name}
        PRIVATE
        cxx_std_23
    )
    if (MSVC)
        target_compile_options(
            ${target_name} 
            PRIVATE
            /W1 
            ) 
    else()
        target_compile_options(
            ${target_name} 
            PRIVATE 
			-w
            >
        )  
    endif()
endfunction()



