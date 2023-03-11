

#function(setup_copy_release_version_of_imported_library_to_output_dir target_name dynamic_library_target)
#	get_target_property(
#        dynamic_library_target_imported_location_release # out value
#        ${dynamic_library_target}
#		IMPORTED_LOCATION_RELEASE
#    )
#	add_custom_command(
#		TARGET ${target_name} POST_BUILD
#		COMMAND "${CMAKE_COMMAND}" -E
#		copy_if_different ${dynamic_library_target_imported_location_release} $<TARGET_FILE_DIR:${target_name}>
#	)
#endfunction()

function(setup_copy_dynamic_library_to_output_directory target_name dynamic_library_target)	

	add_custom_command(
		TARGET ${target_name} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E
		copy_if_different $<TARGET_FILE:${dynamic_library_target}> $<TARGET_FILE_DIR:${target_name}>
	)

	# PREVIOUS VERSION - LESS ROBUST AS IT RELIES ON IMPORTED_LOCATION

	    # get the full path to the dll of the dynamic library we wish to link
	# this assumes:
	# - IMPORTED_CONFIGURATIONS target property is DEBUG and RELEASE
	#
    #get_target_property(
    #    dynamic_library_target_imported_location_debug # out value
    #    ${dynamic_library_target}
	#	IMPORTED_LOCATION_DEBUG
    #)
	#
	#get_target_property(
    #    dynamic_library_target_imported_location_release # out value
    #    ${dynamic_library_target}
	#	IMPORTED_LOCATION_RELEASE
	#	#"$<$<CONFIG:DEBUG>:IMPORTED_LOCATION_DEBUG,IMPORTED_LOCATION>"
	#	#CONFIGURATION $<CONFIG>
    #)
	#
	#set(IS_DEBUG_VARIANT $<CONFIG:Debug>) # An abbreviation to make the example easier to read.
	#set(IS_RELEASE_VARIANT $<NOT:$<CONFIG:Debug>>) # An abbreviation to make the example easier to read.
	#add_custom_command(
	#	TARGET ${target_name} POST_BUILD
	#	COMMAND "${CMAKE_COMMAND}" -E
	#	copy_if_different $<IF:${IS_DEBUG_VARIANT},${dynamic_library_target_imported_location_debug},${dynamic_library_target_imported_location_release}> $<TARGET_FILE_DIR:${target_name}>
	#)
	

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
    
endfunction()


function(setup_copy_runtime_dlls_to_output_directory target_name)	

	add_custom_command(
		TARGET ${target_name} POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E
		copy_if_different $<TARGET_RUNTIME_DLLS:${target_name}> $<TARGET_FILE_DIR:${target_name}>
		COMMAND_EXPAND_LISTS
	)
    
endfunction()

