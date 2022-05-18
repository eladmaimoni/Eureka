



function(set_compiler_flags target_name)
    if (MSVC)
        target_compile_options(
            ${target_name} 
            PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:
            /W4 
            #"$<$<CONFIG:Debug>:/ZI>"
            "$<$<CONFIG:Release>:/WX>"
            "$<$<CONFIG:RELEASE>:/O2>"
            >
            ) # Edit and Continue
        target_link_options(
            ${target_name} 
            PRIVATE 
            $<$<COMPILE_LANGUAGE:CXX>:
            "$<$<CONFIG:Debug>:/INCREMENTAL>"
            #$"$<$<CONFIG:Release>:/LTCG>"
            "$<$<NOT:$<CONFIG:Debug>>:/LTCG>"
            >
            )
    else()
        target_compile_options(
            ${target_name} 
            PRIVATE 
            $<$<COMPILE_LANGUAGE:CXX>:
            "$<$<CONFIG:RELEASE>:-O3>"
            "-Wall"
            "-Wextra"
             "$<$<CONFIG:RELEASE>:-Werror>"
            "$<$<CONFIG:RELEASE>:-O3>"
            >
        )  
    endif()
endfunction()

function(configure_cuda)
    # https://cmake.org/cmake/help/latest/module/FindCUDAToolkit.html#module:FindCUDAToolkit
    find_package(CUDAToolkit)
    if (CUDAToolkit_FOUND)
        message("found cuda toolkit version ${CUDAToolkit_VERSION_MAJOR}.${CUDAToolkit_VERSION_MINOR}.${CUDAToolkit_VERSION_PATCH}")
    endif()
    set( CMAKE_CUDA_COMPILER ${CUDAToolkit_NVCC_EXECUTABLE})
    enable_language(CUDA)  
    set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS}" "-use_fast_math -lineinfo --ptxas-options=-v" )
endfunction()

function (configure_cuda_for_target target_name)
    # following section supposed to detect the cuda architecture
    # https://stackoverflow.com/questions/68223398/how-can-i-get-cmake-to-automatically-detect-the-value-for-cuda-architectures
    message("finding cuda architectures using deprecated module ...")
    include(FindCUDA/select_compute_arch)
    CUDA_DETECT_INSTALLED_GPUS(INSTALLED_GPU_CCS_1)
    string(STRIP "${INSTALLED_GPU_CCS_1}" INSTALLED_GPU_CCS_2)
    string(REPLACE " " ";" INSTALLED_GPU_CCS_3 "${INSTALLED_GPU_CCS_2}")
    string(REPLACE "." "" CUDA_ARCH_LIST "${INSTALLED_GPU_CCS_3}")
    #SET(CMAKE_CUDA_ARCHITECTURES ${CUDA_ARCH_LIST})
    message("cuda achitecture found = ${CMAKE_CUDA_ARCHITECTURES}")
    set_target_properties(${target_name} PROPERTIES CUDA_ARCHITECTURES ${CUDA_ARCH_LIST})


    # https://docs.nvidia.com/cuda/cuda-compiler-driver-nvcc/index.html
    target_compile_options(
    ${target_name} 
    PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:
    "-use_fast_math" 
    "-lineinfo" 
    "--ptxas-options=-v"
    "-err-no"
    #"-maxrregcount=50"
    #"-gencode=arch=compute_80,code=sm_80"
    >
    )

endfunction()

function(add_static_library library_name)
    message("adding static library ${library_name}")
    add_library(${library_name} "${ARGN}")
    set_target_properties(${library_name} PROPERTIES OPTIMIZE_DEPENDENCIES ON)
    set_compiler_flags(${library_name})       
endfunction()

function(add_exe exe_name)
    message("adding executable ${exe_name}")   
    add_executable(${exe_name} "${ARGN}")  
    set_target_properties(${exe_name} PROPERTIES OPTIMIZE_DEPENDENCIES ON)
    set_compiler_flags(${exe_name})  
endfunction()


function(target_link_transitive_includes target_library)    
	foreach(src_library IN LISTS ARGN)
        get_target_property(library_interface_includes ${src_library} INTERFACE_INCLUDE_DIRECTORIES)
        message("${src_library} interface includes: ${library_interface_includes}")
        set_target_properties(${target_library} PROPERTIES INCLUDE_DIRECTORIES "${library_interface_includes}")
    endforeach()
endfunction()

function(target_link_libraries_non_transitive target)
        #[[
        On MSVC, this results in manually linking ONLY the provided libraries in-source
        via #pragma comment(lib, "my_lib.lib") instead of using target_link_libraries.
        this allows to have full control of link dependencies instead of relying
        on CMAKE transitive behaviour which might result in a larger amount of dependencies
        that significantly affect compilation time.
        ]]
    if (MSVC)
    
        # generate the file content
        foreach(lib IN LISTS ARGN)
             message("linking library : ${lib} \n")
             # add the include interface directories
             get_target_property(library_interface_includes ${lib} INTERFACE_INCLUDE_DIRECTORIES)
             set_property(TARGET ${target} APPEND PROPERTY INCLUDE_DIRECTORIES "${library_interface_includes}")
             add_dependencies(${target} ${lib})
             string(APPEND in_source_link_file_content "#pragma comment(lib, \"${lib}.lib\")\n")
        endforeach()
        
        # generate the cpp file containing the pragma comment
        string(APPEND in_source_link_file "${CMAKE_CURRENT_BINARY_DIR}/source_link.cpp")
        file(GENERATE OUTPUT ${in_source_link_file} CONTENT "${in_source_link_file_content}")
        source_group("linker" FILES ${in_source_link_file}) 
        target_sources(${target} PRIVATE ${in_source_link_file})
    else()
        # on other compilers, we simply use target_link_libraries
        target_link_libraries(${target} PRIVATE "${ARGN}")
    endif()
endfunction()


function(dump_cmake_variables)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        if (ARGV0)
            unset(MATCHED)
            string(REGEX MATCH ${ARGV0} MATCHED ${_variableName})
            if (NOT MATCHED)
                continue()
            endif()
        endif()
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endfunction()

macro(set_source_group name)
    set(${name} "${ARGN}")  
    source_group(${name} FILES ${ARGN}) 
endmacro(set_source_group)

function(target_shared_precompiled_headers target_name precompiled_target)
    target_precompile_headers(${target_name} REUSE_FROM precompiled)
    if (MSVC)
        string(APPEND config_str "")
        if (GENERATOR_IS_MULTI_CONFIG)
            string(APPEND config_str "$<CONFIG>/")
        endif()
        # hack to copy .idb file in MSVC Edit & Continue
        get_target_property(pdb_base_src ${precompiled_target} COMPILE_PDB_OUTPUT_DIRECTORY)
        get_target_property(binary_base ${precompiled_target} PRECOMPILED_BINARY_DIR)
        string(APPEND pdb_src "${binary_base}/${pdb_base_src}${config_str}${precompiled_target}.idb")
        string(APPEND pdb_dst "$<TARGET_PROPERTY:${target_name},COMPILE_PDB_OUTPUT_DIRECTORY>${config_str}${precompiled_target}.idb")
        string(
            APPEND copy_idb_cmd
            "$<IF:$<CONFIG:Debug>,"
                "${CMAKE_COMMAND};-E;copy;${pdb_src};${pdb_dst},"
                ""
            ">"
        )
        
        add_custom_command(TARGET ${target_name} PRE_BUILD COMMAND "${copy_idb_cmd}" COMMAND_EXPAND_LISTS)

        
        #source_group(precompiled FILES ${PRECOMPILED_HEADER_FILES})
        string(APPEND hxx_folder "${binary_base}${CMAKE_FILES_DIRECTORY}/${pdb_base_src}$<CONFIG>")
        file (GLOB_RECURSE PRECOMPILED_HEADER_FILES ${hxx_folder}/cmake_pch.hxx)
        source_group(pch FILES ${PRECOMPILED_HEADER_FILES})
         

        file(GENERATE OUTPUT "C:/Projects/$<CONFIG>${target_name}tmp.txt" 
        CONTENT 
        "
        $command: ${copy_idb_cmd}
        ${COMPILE_PDB_OUTPUT_DIRECTORY}  
        ${PRECOMPILED_BINARY_DIR}
        $<CONFIG>
        ${pdb_src}
        ${pdb_dst}
        "
        )

    endif()
endfunction()