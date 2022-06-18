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
    
    struct Image2DUploadTransferDesc
    {
        uint8_t*    src_ptr; // host temporary buffer containing data
        std::size_t bytes;
        vk::Image   destination_image;
    };

    struct BufferDataUploadTransferDesc
    {
        uint8_t* src_ptr; // host temporary buffer containing data
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

        std::vector<Image2DUploadTransferDesc> toStageBufferTransfers;
        toStageBufferTransfers.reserve(gltfModel.images.size() + gltfModel.nodes.size());

        std::vector<SampledImage2D> images;
        images.reserve(gltfModel.images.size());


        std::vector<BufferDataUploadTransferDesc> indicesUploadDesc;
        indicesUploadDesc.reserve(gltfModel.meshes.size());

        std::vector<BufferDataUploadTransferDesc> vertexDataUploadDesc;
        vertexDataUploadDesc.reserve(gltfModel.meshes.size());

        //
        // create images and buffers (no transfer yet)
        //
        std::size_t totalImageMemory = 0;

        for (auto i = 0u; i < gltfModel.images.size(); ++i)
        {
            auto& glTFImage = gltfModel.images[i];
            assert(glTFImage.component == 4 && glTFImage.bits == 8 && glTFImage.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE);
        
            Image2DProperties imageProps
            {
                 .width = static_cast<uint32_t>(glTFImage.width),
                 .height = static_cast<uint32_t>(glTFImage.height),
                 .format = vk::Format::eR8G8B8A8Unorm,
                 .usage_flags = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                 .aspect_flags = vk::ImageAspectFlagBits::eColor,
                 .use_dedicated_memory_allocation = false // unlikely to be resized
            };
            auto& vulkanImage = images.emplace_back(_deviceContext, imageProps);
            toStageBufferTransfers.emplace_back(Image2DUploadTransferDesc{ .src_ptr = glTFImage.image.data(), .bytes = glTFImage.image.size(), .destination_image = vulkanImage.Get()});
            totalImageMemory += glTFImage.image.size();
        }

        const tinygltf::Scene& scene = gltfModel.scenes.at(0);

        for (auto i = 0u; i < scene.nodes.size(); ++i)
        {
            const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
            
            if (node.mesh > -1)
            {
                const tinygltf::Mesh mesh = gltfModel.meshes[node.mesh];

                for (auto j = 0u; j < mesh.primitives.size(); ++j)
                {
                    const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];

                    const tinygltf::Accessor& accessor = gltfModel.accessors[glTFPrimitive.indices];
                    const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];



                }
            }

        }

        co_await concurrencpp::resume_on(*_copySubmitExecutor);

        DEBUGGER_TRACE("rendering thread fun");

        LoadedModel res{};
        co_return res;
    }

    


    void ParsePrimitiveData(
        const tinygltf::Model& gltfModel, 
        const tinygltf::Primitive& glTFPrimitive
    )
    {
        //const tinygltf::Accessor& accessor = gltfModel.accessors[glTFPrimitive.indices];
        //const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
        //const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];

        const float* positionBuffer = nullptr;
        const float* normalsBuffer = nullptr;
        const float* texCoordsBuffer = nullptr;

        // Get buffer data for vertex normals

        assert(glTFPrimitive.attributes.contains("POSITION") && glTFPrimitive.attributes.contains("NORMAL") && glTFPrimitive.attributes.contains("TEXCOORD_0")); // only support this type for now


        auto positionAttr = glTFPrimitive.attributes.find("POSITION");
        auto normalAttr = glTFPrimitive.attributes.find("NORMAL");
        auto texAttr = glTFPrimitive.attributes.find("TEXCOORD_0");
        auto tangentAttr = glTFPrimitive.attributes.find("TANGENT");



        if (positionAttr != glTFPrimitive.attributes.end() && normalAttr != glTFPrimitive.attributes.end() && texAttr != glTFPrimitive.attributes.end())
        {
            const auto& positionAccessor = gltfModel.accessors.at(positionAttr->second);
            const auto& normalAccessor = gltfModel.accessors.at(normalAttr->second);
            const auto& texAccessor = gltfModel.accessors.at(texAttr->second);
        
            if (positionAccessor.bufferView == normalAccessor.bufferView || positionAccessor.bufferView == texAccessor.bufferView || normalAccessor.bufferView == texAccessor.bufferView)
            {
                DEBUGGER_TRACE("interleaved data, not implemented yet"); throw std::logic_error("not implemented");
            }
            
            if (positionAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || normalAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || texAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
            {
                DEBUGGER_TRACE("unsupported component type"); throw std::logic_error("not implemented");
            }

        
            const auto& positionBufferView = gltfModel.bufferViews.at(positionAccessor.bufferView);
            const auto& normalBufferView = gltfModel.bufferViews.at(normalAccessor.bufferView);
            const auto& texBufferView = gltfModel.bufferViews.at(texAccessor.bufferView);

            


            // check if data is interleaved

        }

        //if (glTFPrimitive.attributes.contains("POSITION"))
        //{
        //    

        //    const tinygltf::Accessor& accessor = gltfModel.accessors[glTFPrimitive.attributes.find("POSITION")->second];
        //    const tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
        //    positionBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
        //    vertexCount = accessor.count;
        //}
        //// Get buffer data for vertex normals
        //if (glTFPrimitive.attributes.contains("NORMAL"))
        //{
        //    const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
        //    const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
        //    normalsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
        //}
        //// Get buffer data for vertex texture coordinates
        //// glTF supports multiple sets, we only load the first one
        //if (glTFPrimitive.attributes.contains("TEXCOORD_0"))
        //{
        //    const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
        //    const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
        //    texCoordsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
        //}

    }

} 