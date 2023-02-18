

set_source_group(json 
	"json_nlohmann_specializations.hpp"
	"json_io.hpp"
	"json_serialization_impl.cpp"
)

set_source_group(types 
	"AppTypes.hpp"
)

add_library(Eureka.Serialization ${json} ${types})
set_target_properties(Eureka.Serialization PROPERTIES FOLDER "Libs")


target_include_directories(
    Eureka.Serialization
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    Eureka.Serialization 
    PRIVATE
    nlohmann_json::nlohmann_json
	eureka_strict_compiler_flags
)