#include "AssetLoading.hpp"
#include <tiny_gltf.h>
#include <debugger_trace.hpp>
#include <basic_errors.hpp>
#include <Image.hpp>
#include <Buffer.hpp>

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
    
    struct MemoryTransferDesc
    {
        uint8_t*    src_ptr;
        std::size_t bytes;
    };

    result_t<LoadedModel> AssetLoader::LoadModel(
        const std::filesystem::path& path,
        const ModelLoadingConfig& config
    )
    {
        if (!std::filesystem::exists(path))
        {
            throw file_not_found_error(path);
        }

        auto pathstring = path.string();

        co_await concurrencpp::resume_on(*_ioExecutor);
     
        std::string error;
        std::string warning;
        tinygltf::Model gltfModel;
        tinygltf::TinyGLTF gltfContext;

        DEBUGGER_TRACE("io thread fun");

        auto ok = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, pathstring);

        if (!ok)
        {
            DEBUGGER_TRACE("failed loding {} : error = {} warning = {}", path, error, warning);
            throw std::runtime_error("bad gltf");
        }

        co_await concurrencpp::resume_on(*_poolExecutor);
        
        

        DEBUGGER_TRACE("pool thread fun");

        std::vector<MemoryTransferDesc> toStageBufferTransfers;
        toStageBufferTransfers.reserve(gltfModel.images.size() + gltfModel.nodes.size());

        std::vector<Image2D> images;
        images.reserve(gltfModel.images.size());

        //
        // create images and buffers (no transfer yet)
        //
        std::size_t totalImageMemory = 0;



        for (auto i = 0u; i < gltfModel.images.size(); ++i)
        {
            auto& glTFImage = gltfModel.images[i];
            assert(glTFImage.component == 4 && glTFImage.bits == 8 && glTFImage.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE);
        

            toStageBufferTransfers.emplace_back(MemoryTransferDesc{ .src_ptr = glTFImage.image.data(), .bytes = glTFImage.image.size() });
            totalImageMemory += glTFImage.image.size();

            Image2DProperties imageProps
            {
                 .width = static_cast<uint32_t>(glTFImage.width),
                 .height = static_cast<uint32_t>(glTFImage.height),
                 .format = vk::Format::eR8G8B8A8Unorm,
                 .usage_flags = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                 .aspect_flags = vk::ImageAspectFlagBits::eColor,
                 .use_dedicated_memory_allocation = false // unlikely to be resized
            };
            images.emplace_back(_deviceContext, imageProps);
        }

    



        co_await concurrencpp::resume_on(*_copySubmitExecutor);

        DEBUGGER_TRACE("rendering thread fun");

        LoadedModel res{};
        co_return res;
    }

} 