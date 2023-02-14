
set_source_group(api 
    "FlutterEmbedderAPI.h"
    "FlutterEmbedderAPI.cpp"
) 


add_library(FlutterEmbedderAPI SHARED ${api})

set_target_properties(FlutterEmbedderAPI PROPERTIES FOLDER "Libs")