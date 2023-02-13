function(link_library_and_copy_dynamic_library_to_output_path target_name dynamic_library_target)	
    # get the full path to the dll of the dynamic library we wish to link
	
	
	
    get_target_property(
        dynamic_library_target_imported_location_release # out value
        ${dynamic_library_target}
		IMPORTED_LOCATION
		#"$<$<CONFIG:DEBUG>:IMPORTED_LOCATION_DEBUG,IMPORTED_LOCATION>"
		#CONFIGURATION $<CONFIG>
    )

    get_target_property(
        dynamic_library_target_imported_location_debug # out value
		${dynamic_library_target}
		IMPORTED_LOCATION_DEBUG
    )
	
	
	set(IS_DEBUG_VARIANT $<CONFIG:Debug>) # An abbreviation to make the example easier to read.
	set(IS_RELEASE_VARIANT $<NOT:$<CONFIG:Debug>>) # An abbreviation to make the example easier to read.
	#add_custom_command(
	#	TARGET ${target_name} POST_BUILD
	#	COMMAND "${CMAKE_COMMAND}" -E
	#	# in release build, this is an echo command
	#	$<${IS_RELEASE_VARIANT}:echo>
	#	# output text to make clear that the copy command is not actually issued
	#	# in release build, the echo is 
	#	$<${IS_RELEASE_VARIANT}:"copy omitted for release build, command would have been ">
	#	# the actual copy command, which is overridden by the "echo" above
	#	# in the case of a non-release build
	#	copy_if_different ${dynamic_library_target_imported_location_debug} $<TARGET_FILE_DIR:${target_name}>
	#	)
	#
	#add_custom_command(
	#	TARGET ${target_name} POST_BUILD
	#	COMMAND "${CMAKE_COMMAND}" -E
	#	# in release build, this is an echo command
	#	$<${IS_DEBUG_VARIANT}:echo>
	#	# output text to make clear that the copy command is not actually issued
	#	# in release build, the echo is 
	#	$<${IS_DEBUG_VARIANT}:"copy omitted for debug build, command would have been ">
	#	# the actual copy command, which is overridden by the "echo" above
	#	# in the case of a non-release build
	#	copy_if_different ${dynamic_library_target_imported_location_release} $<TARGET_FILE_DIR:${target_name}>
	#)
	add_custom_command(
		TARGET ${target_name} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E
		copy_if_different $<IF:${IS_DEBUG_VARIANT},${dynamic_library_target_imported_location_debug},${dynamic_library_target_imported_location_release}> $<TARGET_FILE_DIR:${target_name}>
	)
	#if(MSVC)
	#	add_custom_command(TARGET ${target_name} PRE_BUILD
	#		COMMAND cmd.exe if "$(Configuration)" == "Debug" "${CMAKE_COMMAND}" -E copy_if_different "${dynamic_library_target_imported_location_debug}" "$<TARGET_FILE_DIR:${target_name}>" )
	#		
	#	#add_custom_command(TARGET ${target_name} PRE_BUILD
	#	#	COMMAND cmd.exe /c if "$(Configuration)" != "Debug" "${CMAKE_COMMAND}" -E copy_if_different "${dynamic_library_target_imported_location_release}" $<TARGET_FILE_DIR:${target_name}> )
	#endif()
	
	
	
	#add_custom_command(TARGET ${target_name} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E echo 
    # ">>>>>>>>>>>>>>>>>> IMPORTED_LOCATION = $<$<CONFIG:Debug>:IMPORTED_LOCATION_DEBUG,IMPORTED_LOCATION>")

    #message("dynamic_library_target_imported_location = ${dynamic_library_target_imported_location}")

    #add_custom_command(
    #    TARGET 
    #    ${target_name}  # the target that uses the dll 
    #    PRE_BUILD       # this command is executed as a pre build step for the target
    #    COMMAND 
    #    ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
    #    #"${dynamic_library_target_imported_location}"  # <--this is in-file
	#	"$<$<CONFIG:DEBUG>:${dynamic_library_target_imported_location_debug},${dynamic_library_target_imported_location_release}>"
    #    $<TARGET_FILE_DIR:${target_name}>   # <--this is out-file path
    #)    
    
    target_link_libraries(
        ${target_name}
        PRIVATE
        ${dynamic_library_target}
    )
endfunction()

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

function(set_compiler_flags target_name)
    target_compile_features(
        ${target_name}
        PRIVATE
        cxx_std_23
    )
    if (MSVC)
        target_compile_options(
            ${target_name} 
            PRIVATE
            /W4 
            ) 
        target_link_options(
            ${target_name} 
            PRIVATE 
            )
    else()
        target_compile_options(
            ${target_name} 
            PRIVATE 
            $<$<COMPILE_LANGUAGE:CXX>:
            "-Wall"
            "-Wextra"
             "$<$<CONFIG:RELEASE>:-Werror>"
            >
        )  
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

macro(configure_default_output_directories)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(EUREKA_COMPILED_SHADERS_DIRECTORY ${CMAKE_BINARY_DIR}/compiled_shaders)
	set(EUREKA_AUTOGENERATED_DIRECTORY ${CMAKE_BINARY_DIR}/autogenerated)
    #message("===================================================================")
    #message("CMAKE_ARCHIVE_OUTPUT_DIRECTORY = ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
    #message("CMAKE_LIBRARY_OUTPUT_DIRECTORY = ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
    #message("CMAKE_RUNTIME_OUTPUT_DIRECTORY = ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    #message("===================================================================")

endmacro()

#####################################################################################
#####################################################################################
########################       GET AVAILABLE TARGETS          #######################
#####################################################################################
#####################################################################################
function(get_all_targets var)
    set(targets)
    get_all_targets_recursive(targets ${CMAKE_CURRENT_SOURCE_DIR})
    set(${var} ${targets} PARENT_SCOPE)
endfunction()

macro(get_all_targets_recursive targets dir)
    get_property(subdirectories DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
    foreach(subdir ${subdirectories})
        get_all_targets_recursive(${targets} ${subdir})
    endforeach()

    get_property(current_targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
    list(APPEND ${targets} ${current_targets})
endmacro()

#get_all_targets(all_targets)
#message("All targets: ${all_targets}")



