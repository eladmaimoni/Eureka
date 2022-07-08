#include "AssetLoading.hpp"
#include <tiny_gltf.h>
#include <debugger_trace.hpp>
#include <basic_errors.hpp>
#include <Image.hpp>
#include <Buffer.hpp>
#include <basic_utils.hpp>


namespace eureka
{
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
    

    AssetLoader::AssetLoader(
        DeviceContext& deviceContext, 
        Queue queue, 
        std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext,
        std::shared_ptr<OneShotCopySubmissionHandler>     oneShotCopySubmissionHandler,
        std::shared_ptr<HostWriteCombinedRingPool>        uploadPool,
        IOExecutor ioExecutor, PoolExecutor poolExecutor
    ) :
        _deviceContext(deviceContext),
        _copyQueue(queue),
        _submissionThreadExecutionContext(std::move(submissionThreadExecutionContext)),
        _oneShotCopySubmissionHandler(std::move(oneShotCopySubmissionHandler)),
        _ioExecutor(std::move(ioExecutor)),
        _poolExecutor(std::move(poolExecutor)),
        _uploadPool(std::move(uploadPool)),
        //_stageZone(deviceContext, StageZoneConfig{ .bytes_capacity = STAGE_ZONE_SIZE }),
        _uploadCommandPool(deviceContext.LogicalDevice(), CommandPoolDesc{ .type = CommandPoolType::eTransientResettableBuffers, .queue_family = _copyQueue.Family() })
    {

    }

    void ThrowOnCancelled(const std::stop_token& stopToken)
    {
        if (stopToken.stop_requested())
        {
            throw operation_cancelled("cancelled");
        }
    }

    vkr::CommandBuffer AssetLoader::RecordUploadCommands(
        dynamic_span<ImageStageUploadDesc> imageUploads,
        const BufferDataUploadTransferDesc& bufferUpload,
        const PoolSequentialStageZone& stageZone    
    )
    {
        PROFILE_CATEGORIZED_SCOPE("Asset Loading Command Recording", Profiling::Color::Green, Profiling::PROFILING_CATEGORY_RENDERING);
        DEBUGGER_TRACE("rendering thread fun - recording one shot copy");

        auto uploadCommandBuffer = _submissionThreadExecutionContext->OneShotCopySubmitCommandPool().AllocatePrimaryCommandBuffer();

        auto& copyQueue = _submissionThreadExecutionContext->CopyQueue();
        auto& graphicsQueue = _submissionThreadExecutionContext->GraphicsQueue();

        {
            ScopedCommands commands(uploadCommandBuffer);

            svec10<vk::ImageMemoryBarrier> preTransferImageMemoryBarriers;
            svec10<vk::ImageMemoryBarrier> postTransferImageMemoryBarriers;
            svec10<vk::BufferImageCopy> bufferImageCopies;

            for (const auto& imageUploadDesc : imageUploads)
            {
                auto [preTransferBarrier, bufferImageCopy, postTransferBarrier]
                    = ShaderSampledImageUploadTuple(_copyQueue, graphicsQueue, imageUploadDesc);

                preTransferImageMemoryBarriers.emplace_back(preTransferBarrier);
                bufferImageCopies.emplace_back(bufferImageCopy);
                postTransferImageMemoryBarriers.emplace_back(postTransferBarrier);
            }
      
            uploadCommandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eTransfer,
                {},
                nullptr,
                nullptr,
                preTransferImageMemoryBarriers
            );

            for (auto i = 0u; i < imageUploads.size(); ++i)
            {
                uploadCommandBuffer.copyBufferToImage(
                    stageZone.Buffer(),
                    imageUploads[i].destination_image,
                    vk::ImageLayout::eTransferDstOptimal,
                    { bufferImageCopies[i] }
                );
            }

            uploadCommandBuffer.copyBuffer(
                bufferUpload.src_buffer,
                bufferUpload.dst_buffer,
                { vk::BufferCopy{.srcOffset = bufferUpload.src_offset, .dstOffset = bufferUpload.dst_offset, .size = bufferUpload.bytes} }
            );

            vk::BufferMemoryBarrier bufferMemoryBarrier
            {
                 .srcAccessMask = vk::AccessFlagBits::eTransferWrite,
                 .dstAccessMask = vk::AccessFlagBits::eShaderRead,
                 .srcQueueFamilyIndex = copyQueue.Family(),
                 .dstQueueFamilyIndex = graphicsQueue.Family(),
                 .buffer = bufferUpload.dst_buffer,
                 .offset = 0,
                 .size = bufferUpload.bytes
            };

            uploadCommandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eBottomOfPipe,
                {},
                nullptr,
                { bufferMemoryBarrier },
                postTransferImageMemoryBarriers
            );
        }
        return uploadCommandBuffer;
    }

    future_t<LoadedModel> AssetLoader::LoadModel(
        const std::filesystem::path& path,
        const ModelLoadingConfig& config
    )
    {
        PROFILE_CATEGORIZED_UNTHREADED_SCOPE("Asset Loading Background", Profiling::Color::Red, Profiling::PROFILING_CATEGORY_LOAD);
        auto cancellationToken = config.cancel;
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

        ThrowOnCancelled(cancellationToken);

        co_await concurrencpp::resume_on(*_poolExecutor);

        DEBUGGER_TRACE("pool thread fun");

        svec10<ImageStageUploadDesc> imageUploadDescs;
        imageUploadDescs.reserve(gltfModel.images.size() + gltfModel.nodes.size());

        svec10<SampledImage2D> images;
        images.reserve(gltfModel.images.size());


        svec10<dynamic_cspan<uint8_t>> indicesUploadDesc;
        indicesUploadDesc.reserve(gltfModel.meshes.size());

        svec10<dynamic_cspan<uint8_t>> vertexDataUploadDesc;
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
            imageUploadDescs.emplace_back(
                ImageStageUploadDesc
                { 
                    .unpinned_src_span = dynamic_cspan<uint8_t>(glTFImage.image.data(), glTFImage.image.size()),
                    .stage_zone_offset = totalImageMemory,
                    .destination_image = vulkanImage.Get(),
                    .destination_image_extent = vk::Extent3D{.width = imageProps.width, .height = imageProps.width, .depth = 1}
                }
            );
            totalImageMemory += glTFImage.image.size();
        }

        const tinygltf::Scene& scene = gltfModel.scenes.at(0);

        std::size_t totalBufferMemory = 0;

        for (auto i = 0u; i < scene.nodes.size(); ++i)
        {
            const tinygltf::Node gltfNode = gltfModel.nodes[scene.nodes[i]];
            
            if (gltfNode.mesh > -1)
            {
                const tinygltf::Mesh mesh = gltfModel.meshes[gltfNode.mesh];

                for (auto j = 0u; j < mesh.primitives.size(); ++j)
                {
                    auto primitiveView = ExtractPrimitiveData(gltfModel, mesh.primitives[j]);

                    // TODO: calculate buffer offsets
                    totalBufferMemory += indicesUploadDesc.emplace_back(to_raw_span(primitiveView.index_view)).size_bytes();
                    totalBufferMemory += vertexDataUploadDesc.emplace_back(to_raw_span(primitiveView.position_view)).size_bytes();
                    totalBufferMemory += vertexDataUploadDesc.emplace_back(to_raw_span(primitiveView.normal_view)).size_bytes();
                    totalBufferMemory += vertexDataUploadDesc.emplace_back(to_raw_span(primitiveView.uv_view)).size_bytes();
                    totalBufferMemory += vertexDataUploadDesc.emplace_back(to_raw_span(primitiveView.tangent_view)).size_bytes();
                }
            }
        }

        VertexAndIndexTransferableDeviceBuffer deviceBuffer(_deviceContext.Allocator(), BufferConfig{ .byte_size = totalBufferMemory });

 
        auto stageBuffer = co_await _uploadPool->EnqueueAllocation(totalBufferMemory + totalImageMemory);

        DEBUGGER_TRACE("stage buffer allocated");

        PoolSequentialStageZone stageZone(std::move(stageBuffer));

    
        //_stageZone.Reset(); 
        // TODO check stage zone leftover
        for (const auto& imageUploadDesc : imageUploadDescs)
        {      
            stageZone.Assign(imageUploadDesc.unpinned_src_span);
        }

        BufferDataUploadTransferDesc bufferUploadDesc
        {
            .src_buffer = stageZone.Buffer(),
            .src_offset = stageZone.Position(),
            .bytes = deviceBuffer.ByteSize(),
            .dst_buffer = deviceBuffer.Buffer(),
            .dst_offset = 0
        };
     
        for (const auto& idxSpan : indicesUploadDesc)
        {
            stageZone.Assign(idxSpan);
        }
        for (const auto& vertexSpan : vertexDataUploadDesc)
        {
            stageZone.Assign(vertexSpan);
        }

        ThrowOnCancelled(cancellationToken);
  
        co_await concurrencpp::resume_on(_submissionThreadExecutionContext->OneShotCopySubmitExecutor());

        auto uploadCommandBuffer = RecordUploadCommands(imageUploadDescs, bufferUploadDesc, stageZone);

        co_await _oneShotCopySubmissionHandler->AppendOneShotCommandBufferSubmission(std::move(uploadCommandBuffer));
        

        DEBUGGER_TRACE("rendering thread fun - copy submitted and signaled as done");

        co_await concurrencpp::resume_on(*_poolExecutor); // temporary, release resources on pool thread
        LoadedModel res{};

        DEBUGGER_TRACE("loading done");
        co_return res;
    }

    




} 