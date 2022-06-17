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
    

    result_t<LoadedModel> AssetLoader::LoadModel(
        const std::filesystem::path& path,
        const ModelLoadingConfig& config
    )
    {


  
  

     
     
        if (!std::filesystem::exists(path))
        {
            throw file_not_found_error(path);
        }

        auto modelResult = _ioExecutor->submit(
            [pathstring = path.string()]()
            {
                std::string error;
                std::string warning;
                tinygltf::Model gltfModel;
                tinygltf::TinyGLTF gltfContext;

                DEBUGGER_TRACE("io thread fun");

                auto ok = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, pathstring);

                if (!ok)
                {
                    DEBUGGER_TRACE("failed loding {} : error = {} warning = {}", pathstring, error, warning);
                    throw std::runtime_error("bad gltf");
                }

                return gltfModel;
            }
        
        );
        auto model = co_await modelResult;

        co_await concurrencpp::resume_on(*_poolExecutor);
        
        DEBUGGER_TRACE("pool thread fun");

        co_await concurrencpp::resume_on(*_copySubmitExecutor);

        DEBUGGER_TRACE("rendering thread fun");

        LoadedModel res{};
        co_return res;
    }

} 