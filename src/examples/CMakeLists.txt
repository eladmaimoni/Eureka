if (WIN32) # until we compile flutter engine for linux
	add_subdirectory(FlutterAppJIT)
	add_subdirectory(FlutterAppAOT)
endif()

#set_target_properties(FlutterAppJIT PROPERTIES FOLDER "Examples")
