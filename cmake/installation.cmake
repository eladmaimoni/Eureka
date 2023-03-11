include(CMakePackageConfigHelpers)

set(EUREKA_INSTALL_CONFIG $<IF:$<CONFIG:Debug>,Debug,Release>)

function(setup_target_includes_for_install 
    target_name
    public_include_folder # the public include folder
    )
    target_include_directories(
        ${target_name}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/> # when using within the build, the include directories is the current source dir
        $<INSTALL_INTERFACE:include/> # when used from installation folder, the include directories is the exported path
    )

    # when installing, copy the public header files to the include directory
    file(GLOB_RECURSE 
        public_include_list 
        "${public_include_folder}/*.h" 
        "${public_include_folder}/*.hpp"
    )
    
    install(FILES ${public_include_list} DESTINATION include/${public_include_folder})

    message(STATUS "public include list ${public_include_list}")
endfunction()

function(setup_target_for_find_package 
    target_name 
    )
    message("ZZZZZZZZZZZ configure_package_config_file ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Config.cmake.in -> ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.cmake")
    
    
    # when installing, generate a "targets" file. clients can include this
    install(
        EXPORT ${target_name}
        FILE ${target_name}-targets.cmake
        DESTINATION cmake
    )

    # generate a "config" file that includes the generated target file cmake file
    # this is for 'find_package' to work
    configure_package_config_file(
      ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Config.cmake.in # input template
      ${CMAKE_CURRENT_BINARY_DIR}/${target_name}-config.cmake # output file
      INSTALL_DESTINATION cmake
      NO_SET_AND_CHECK_MACRO
      NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

    install(
      FILES
      ${CMAKE_CURRENT_BINARY_DIR}/${target_name}-config.cmake
      DESTINATION cmake
      )
endfunction()

function(setup_target_compiled_artifacts_for_install 
    target_name 
    )
    # when installing, 
    # - copy the target artifacts (in this case .lib & .dll) to the lib directory withing the install path
    # - create a cmake files that defines cmake targets with all relevant stuff that clients could link to

    # this causes the dependant runtime artifacts such as dlls 
    # to be copied into the bin directory
    
    install(
        TARGETS ${target_name}
        EXPORT ${target_name}  # when installing, generate a "targets" file. clients can include this
        #DESTINATION lib/$<CONFIG>
        ARCHIVE DESTINATION lib/${EUREKA_INSTALL_CONFIG}
        RUNTIME DESTINATION bin/${EUREKA_INSTALL_CONFIG}
        #RUNTIME_DEPENDENCY_SET FLUTTER_EMBEDDER_API_RUNTIME_DEPENDENCY_SET
        #DESTINATION lib/${EUREKA_INSTALL_CONFIG}
        )
    
    # this piece of SHIT does not generate correct IMPORTED_CONFIGURATIONS. WHY????
    #install(
    #    TARGETS ${target_name}
    #    EXPORT ${target_name}
    #    CONFIGURATIONS Debug
    #    RUNTIME DESTINATION bin/Debug
    #    ARCHIVE DESTINATION lib/Debug
    #)
    #
    #install(
    #    TARGETS ${target_name}
    #    CONFIGURATIONS Release RelWithDebInfo
    #    RUNTIME DESTINATION bin/Release
    #    ARCHIVE DESTINATION lib/Release
    #)

    get_target_property(target_type ${target_name} TYPE)

    # PDB FILES

    if(target_type STREQUAL STATIC_LIBRARY)
        install(
            #FILES $<TARGET_COMPILE_PDB_FILE:${target_name}> # https://gitlab.kitware.com/cmake/cmake/-/issues/16935
            FILES $<TARGET_FILE_DIR:${target_name}>/${target_name}.pdb 
            DESTINATION lib/${EUREKA_INSTALL_CONFIG} OPTIONAL
        )
    
    elseif(        
        target_type STREQUAL SHARED_LIBRARY OR 
        target_type STREQUAL EXECUTABLE
        )
    
        install(
            FILES $<TARGET_PDB_FILE:${target_name}>      
            DESTINATION lib/${EUREKA_INSTALL_CONFIG} OPTIONAL
        )
    
        #install(
        #  IMPORTED_RUNTIME_ARTIFACTS
        #  ${target_name}
        #  DESTINATION bin/$<CONFIG>
        #  )
    endif()
endfunction()

function(setup_target_for_install 
    target_name 
    public_include_folder # the public include folder
    )
    
    setup_target_includes_for_install(${target_name} ${public_include_folder})
    setup_target_for_find_package(${target_name})
    setup_target_compiled_artifacts_for_install(${target_name})
endfunction()
