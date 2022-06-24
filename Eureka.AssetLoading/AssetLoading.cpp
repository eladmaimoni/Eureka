#include "AssetLoading.hpp"
#include <tiny_gltf.h>
#include <debugger_trace.hpp>
#include <basic_errors.hpp>
#include <Image.hpp>
#include <Buffer.hpp>
#include <basic_utils.hpp>


namespace eureka
{
    template<typename T>
    using dynamic_span = std::span<T, std::dynamic_extent>;

    template<typename T>
    using dynamic_cspan = std::span<const T, std::dynamic_extent>;

    template<typename T>
    dynamic_cspan<uint8_t> to_raw_span(const dynamic_cspan<T>& s)
    {
        return dynamic_cspan<uint8_t>(
            reinterpret_cast<const uint8_t*>(s.data()),
            s.size_bytes()
            );
    }

    struct PrimitiveDataView
    {
        dynamic_cspan<uint16_t> index_view;
        dynamic_cspan<float>    position_view;
        dynamic_cspan<float>    normal_view;
        dynamic_cspan<float>    uv_view;
        dynamic_cspan<float>    tangent_view;
    };

    PrimitiveDataView ExtractPrimitiveData(
        const tinygltf::Model& gltfModel,
        const tinygltf::Primitive& glTFPrimitive
    )
    {
        auto positionAttr = glTFPrimitive.attributes.find("POSITION");
        auto normalAttr = glTFPrimitive.attributes.find("NORMAL");
        auto texAttr = glTFPrimitive.attributes.find("TEXCOORD_0");
        auto tangentAttr = glTFPrimitive.attributes.find("TANGENT");

        if (any_equal_to_first(glTFPrimitive.attributes.end(), positionAttr, normalAttr, texAttr, tangentAttr))
        {
            DEBUGGER_TRACE("missing data, not implemented yet"); throw std::logic_error("not implemented");
        }

        const auto& indexAccessor = gltfModel.accessors[glTFPrimitive.indices];
        const auto& positionAccessor = gltfModel.accessors.at(positionAttr->second);
        const auto& normalAccessor = gltfModel.accessors.at(normalAttr->second);
        const auto& texAccessor = gltfModel.accessors.at(texAttr->second);
        const auto& tangentAccessor = gltfModel.accessors.at(tangentAttr->second);

        if (any_pair_is_equal(positionAccessor.bufferView, normalAccessor.bufferView, texAccessor.bufferView, tangentAccessor.bufferView))
        {
            DEBUGGER_TRACE("interleaved gltf data, not implemented yet"); throw std::logic_error("not implemented");
        }

        if (any_not_equal_to_first(TINYGLTF_COMPONENT_TYPE_FLOAT, positionAccessor.componentType, normalAccessor.componentType, texAccessor.componentType, tangentAccessor.componentType))
        {
            DEBUGGER_TRACE("unsupported component type"); throw std::logic_error("not implemented");
        }

        if (indexAccessor.componentType != TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT)
        {
            DEBUGGER_TRACE("unsupported component type"); throw std::logic_error("not implemented");
        }

        const auto& indexBufferView = gltfModel.bufferViews[indexAccessor.bufferView];
        const auto& positionBufferView = gltfModel.bufferViews.at(positionAccessor.bufferView);
        const auto& normalBufferView = gltfModel.bufferViews.at(normalAccessor.bufferView);
        const auto& texBufferView = gltfModel.bufferViews.at(texAccessor.bufferView);
        const auto& tangentBufferView = gltfModel.bufferViews.at(tangentAccessor.bufferView);

        return PrimitiveDataView
        {
            .index_view = std::span(reinterpret_cast<const uint16_t*>(gltfModel.buffers[indexBufferView.buffer].data.data() + indexBufferView.byteOffset + indexAccessor.byteOffset), indexAccessor.count * tinygltf::GetNumComponentsInType(indexAccessor.type)),
            .position_view = std::span(reinterpret_cast<const float*>(gltfModel.buffers[positionBufferView.buffer].data.data() + positionBufferView.byteOffset + positionAccessor.byteOffset), positionAccessor.count * tinygltf::GetNumComponentsInType(positionAccessor.type)),
            .normal_view = std::span(reinterpret_cast<const float*>(gltfModel.buffers[normalBufferView.buffer].data.data() + normalBufferView.byteOffset + normalAccessor.byteOffset), normalAccessor.count * tinygltf::GetNumComponentsInType(normalAccessor.type)),
            .uv_view = std::span(reinterpret_cast<const float*>(gltfModel.buffers[texBufferView.buffer].data.data() + texBufferView.byteOffset + texAccessor.byteOffset), texAccessor.count * tinygltf::GetNumComponentsInType(texAccessor.type)),
            .tangent_view = std::span(reinterpret_cast<const float*>(gltfModel.buffers[tangentBufferView.buffer].data.data() + tangentBufferView.byteOffset + tangentAccessor.byteOffset), tangentAccessor.count * tinygltf::GetNumComponentsInType(tangentAccessor.type))
        };
    }
    
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


    AssetLoader::AssetLoader(
        DeviceContext& deviceContext, 
        Queue queue, 
        std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext,
        IOExecutor ioExecutor, PoolExecutor poolExecutor
    ) :
        _deviceContext(deviceContext),
        _copyQueue(queue),
        _submissionThreadExecutionContext(std::move(submissionThreadExecutionContext)),
        _ioExecutor(std::move(ioExecutor)),
        _poolExecutor(std::move(poolExecutor)),
        _stageZone(deviceContext, StageZoneConfig{ .bytes_capacity = STAGE_ZONE_SIZE }),
        _uploadCommandPool(deviceContext.LogicalDevice(), CommandPoolDesc{ .type = CommandPoolType::eTransientResettableBuffers, .queue_family = _copyQueue.Family() })
    {

    }

    result_t<LoadedModel> AssetLoader::LoadModel(
        const std::filesystem::path& path,
        const ModelLoadingConfig& config
    )
    {
        bool expected = false;
        if (!_busy.compare_exchange_strong(expected, true))
        {
            throw std::logic_error("busy");
        }

        scoped_raii scopedBusy([this]() { _busy.store(false); });


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
            DEBUGGER_TRACE("failed loading {} : error = {} warning = {}", path, error, warning);
            throw std::runtime_error("bad gltf");
        }

        co_await concurrencpp::resume_on(*_poolExecutor);
        
        

        DEBUGGER_TRACE("pool thread fun");

        std::vector<Image2DUploadTransferDesc> toStageBufferTransfers;
        toStageBufferTransfers.reserve(gltfModel.images.size() + gltfModel.nodes.size());

        std::vector<SampledImage2D> images;
        images.reserve(gltfModel.images.size());


        std::vector<dynamic_cspan<uint8_t>> indicesUploadDesc;
        indicesUploadDesc.reserve(gltfModel.meshes.size());

        std::vector<dynamic_cspan<uint8_t>> vertexDataUploadDesc;
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
                    auto primitiveView = ExtractPrimitiveData(gltfModel, mesh.primitives[j]);

                    // TODO: calculate buffer offsets
                    indicesUploadDesc.emplace_back(to_raw_span(primitiveView.index_view));
                    vertexDataUploadDesc.emplace_back(to_raw_span(primitiveView.position_view));
                    vertexDataUploadDesc.emplace_back(to_raw_span(primitiveView.normal_view));
                    vertexDataUploadDesc.emplace_back(to_raw_span(primitiveView.uv_view));
                    vertexDataUploadDesc.emplace_back(to_raw_span(primitiveView.tangent_view));
                }
            }
        }

        

        _stageZone.Reset();

        // TODO check stage zone leftover
        for (const auto& idxSpan : indicesUploadDesc)
        {
            _stageZone.Assign(idxSpan);
        }
        for (const auto& vertexSpan : vertexDataUploadDesc)
        {
            _stageZone.Assign(vertexSpan);
        }
        
        VertexAndIndexTransferableDeviceBuffer deviceBuffer(_deviceContext, BufferConfig{ .byte_size = _stageZone.Position() });


        co_await concurrencpp::resume_on(_submissionThreadExecutionContext->OneShotCopySubmitExecutor());

        // record commands on the main thread for now
        // we should only offload recording to other threads once it is necessary

        // send the command buffer to the copy submit executor which will submit recorded copy commands at once



        DEBUGGER_TRACE("rendering thread fun");

        LoadedModel res{};
        co_return res;
    }

    


} 