add_subdirectory(ImGuiApp)

if (WIN32)
	set(EUREKA_FLUTTER_IS_A_SUBDIRECTORY TRUE)
	#add_subdirectory(eureka_flutter/windows)
endif()