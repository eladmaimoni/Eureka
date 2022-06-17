#include "AssetLoading.hpp"
#include <tiny_gltf.h>
#include <debugger_trace.hpp>
#include <basic_errors.hpp>


//#ifndef VK_CHECK
//#ifdef NDEBUG
//#define VK_CHECK(stmt) eureka::CheckVulkan(stmt); 
//#else
//#define VK_CHECK(stmt) eureka::CheckVulkan(stmt, #stmt, __FILE__, __LINE__); 
//#endif
//#endif

namespace eureka
{
    //void CheckTinyGLTF(bool res, const char* stmt, const char* fname, int line);
    //void CheckTinyGLTF(bool res);
    

    LoadedModel AssetLoader::LoadModel(
        const std::filesystem::path& path,
        const ModelLoadingConfig& config
    )
    {
        tinygltf::Model gltfModel;
        tinygltf::TinyGLTF gltfContext;

        std::string error;
        std::string warning;
     
        if (!std::filesystem::exists(path))
        {
            throw file_not_found_error(path);
        }

        auto ok = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, path.string());

        


        LoadedModel model{};
        return model;
    }

}