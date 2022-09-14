#include "AssetLoading.hpp"
#include <tiny_gltf.h>
#include <debugger_trace.hpp>
#include <basic_errors.hpp>
#include "Image.hpp"
#include "Buffer.hpp"
#include <basic_utils.hpp>
#include <profiling.hpp>

namespace eureka
{


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


        if (tinygltf::GetNumComponentsInType(positionAccessor.type) != 3)
        {
            DEBUGGER_TRACE("unsupported component type"); throw std::logic_error("not implemented");
        }
        if (tinygltf::GetNumComponentsInType(normalAccessor.type) != 3)
        {
            DEBUGGER_TRACE("unsupported component type"); throw std::logic_error("not implemented");
        }        
        if (tinygltf::GetNumComponentsInType(tangentAccessor.type) != 4)
        {
            DEBUGGER_TRACE("unsupported component type"); throw std::logic_error("not implemented");
        }        
        if (tinygltf::GetNumComponentsInType(texAccessor.type) != 2)
        {
            DEBUGGER_TRACE("unsupported component type"); throw std::logic_error("not implemented");
        }
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
        std::shared_ptr<AsyncDataLoader>          asyncDataLoader,
        std::shared_ptr<PipelineCache >           pipelineCache,
        std::shared_ptr<MTDescriptorAllocator>    descPool,
        IOExecutor ioExecutor, PoolExecutor        poolExecutor
    ) :
        _deviceContext(deviceContext),
        _copyQueue(queue),
        _descPool(std::move(descPool)), 
        _pipelineCache(std::move(pipelineCache)),
        _asyncDataLoader(std::move(asyncDataLoader)),
        _ioExecutor(std::move(ioExecutor)),
        _poolExecutor(std::move(poolExecutor)),
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



    

    PreparedModelImages AssetLoader::PrepareImages(tinygltf::Model& gltfModel)
    {
        PreparedModelImages preparedImages{};
        preparedImages.upload_descriptors.reserve(gltfModel.images.size());
        preparedImages.device_images.reserve(gltfModel.images.size());

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
            auto& vulkanImage = preparedImages.device_images.emplace_back(_deviceContext, imageProps);
            preparedImages.upload_descriptors.emplace_back(
                ImageStageUploadDesc
                {
                    .unpinned_src_span = dynamic_cspan<uint8_t>(glTFImage.image.data(), glTFImage.image.size()),
                    .stage_zone_offset = preparedImages.total_image_memory,
                    .destination_image = vulkanImage.Get(),
                    .destination_image_extent = vk::Extent3D{.width = imageProps.width, .height = imageProps.width, .depth = 1}
                }
            );
            preparedImages.total_image_memory += glTFImage.image.size();
        }
        return preparedImages;
    }

   

    future_t<LoadedModel> AssetLoader::LoadModel(
        const std::filesystem::path& path,
        const ModelLoadingConfig& config
    )
    {
        PROFILE_CATEGORIZED_UNTHREADED_SCOPE("Asset Loading Background", eureka::profiling::Color::Red, eureka::profiling::PROFILING_CATEGORY_LOAD);
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

        //DEBUGGER_TRACE("io thread fun");

        auto ok = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, pathstring);

        if (!ok)
        {
            DEBUGGER_TRACE("failed loading {} : error = {} warning = {}", path, error, warning);
            throw file_load_error("bad gltf");
        }

        ThrowOnCancelled(cancellationToken);

        co_await concurrencpp::resume_on(*_poolExecutor);

        DEBUGGER_TRACE("pool thread fun");

 
        svec10<dynamic_cspan<uint8_t>> indicesUploadDesc;
        indicesUploadDesc.reserve(gltfModel.meshes.size());

        svec10<dynamic_cspan<uint8_t>> vertexDataUploadDesc;
        vertexDataUploadDesc.reserve(gltfModel.meshes.size());

        auto pipeline = _pipelineCache->GetPhongShadedMeshWithNormalMapPipeline();
        auto descLayout = _pipelineCache->GetColorAndNormalMapFragmentDescriptorSetLayout().Get();


        auto [totalImageMemory, imageUploadDescs, deviceImages] = PrepareImages(gltfModel);
     
        TexturedPrimitiveGroup primitiveGroup;
        auto& primitiveNodes = primitiveGroup.nodes;


        const tinygltf::Scene& scene = gltfModel.scenes.at(0);

        uint64_t totalIndexBufferMemory = 0;
        uint64_t totalVertexBufferMemory = 0;

        for (auto i = 0u; i < scene.nodes.size(); ++i)
        {
            const tinygltf::Node gltfNode = gltfModel.nodes[scene.nodes[i]];
            
            if (gltfNode.mesh > -1)
            {         
                const tinygltf::Mesh& mesh = gltfModel.meshes[gltfNode.mesh];

                for (auto j = 0u; j < mesh.primitives.size(); ++j)
                {
                    const auto& primitive = mesh.primitives[j];
                    auto primitiveView = ExtractPrimitiveData(gltfModel, primitive);
           
                    // materials 

                    const auto& material = gltfModel.materials[primitive.material];
                    auto colorMapIdx = material.values.at("baseColorTexture").TextureIndex();
                    auto normalMapIdx = material.additionalValues.at("normalTexture").TextureIndex();
         
                    auto materialDecriptorSet = _descPool->AllocateSet(descLayout);

                    std::array<vk::DescriptorImageInfo, 2> imageInfos;
                    auto& colorMapImage = deviceImages[colorMapIdx];
                    auto& normalMapImage = deviceImages[normalMapIdx];

                    imageInfos[0] = vk::DescriptorImageInfo
                    {
                       .sampler = colorMapImage.GetSampler(),
                       .imageView = colorMapImage.GetView(),
                       .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
                    };
                    imageInfos[1] = vk::DescriptorImageInfo
                    {
                       .sampler = normalMapImage.GetSampler(),
                       .imageView = normalMapImage.GetView(),
                       .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
                    };

                    CNTexturedPrimitiveNode primitiveNode{};
                    primitiveNode.fragment_desc_set = std::move(materialDecriptorSet);
                    primitiveNode.fragment_desc_set.SetBindings(0, vk::DescriptorType::eCombinedImageSampler, imageInfos);
                    primitiveNode.buffer_offsets = PreparePrimitiveVertexData(primitiveView, totalIndexBufferMemory, indicesUploadDesc, totalVertexBufferMemory, vertexDataUploadDesc);;

                    primitiveNodes.emplace_back(std::move(primitiveNode));
                }
            }
        }


        primitiveGroup.buffer = VertexAndIndexTransferableDeviceBuffer(_deviceContext.Allocator(), BufferConfig{ .byte_size = totalIndexBufferMemory + totalVertexBufferMemory });
     

        BufferDataUploadTransferDesc bufferUploadDesc
        {
            
            .stage_zone_offset = totalImageMemory,
            .bytes = primitiveGroup.buffer.ByteSize(),
            .dst_buffer = primitiveGroup.buffer.Buffer(),
            .dst_offset = 0
        };

        co_await _asyncDataLoader->UploadImagesAndBufferAsync(
            std::move(imageUploadDescs),
            totalImageMemory,
            std::move(bufferUploadDesc)
            );

        //auto stageBuffer = co_await _uploadPool->EnqueueAllocation(totalIndexBufferMemory + totalVertexBufferMemory + totalImageMemory);

        //DEBUGGER_TRACE("stage buffer allocated");



        //PoolSequentialStageZone stageZone(std::move(stageBuffer));

    
        //_stageZone.Reset(); 
        // TODO check stage zone leftover
        //for (const auto& imageUploadDesc : imageUploadDescs)
        //{      
        //    stageZone.Assign(imageUploadDesc.unpinned_src_span);
        //}


     
        //bufferUploadDesc.unpinned_src_spans = std::move(indicesUploadDesc);

        ////for (const auto& idxSpan : indicesUploadDesc)
        ////{
        ////    stageZone.Assign(idxSpan);
        ////}
        //for (const auto& vertexSpan : vertexDataUploadDesc)
        //{
        //    bufferUploadDesc.unpinned_src_spans.emplace_back(vertexSpan);
        //    //stageZone.Assign(vertexSpan);
        //}

        //ThrowOnCancelled(cancellationToken);
  
        //co_await _oneShotSubmissionHandler->ResumeOnRecordingContext();

        //auto uploadCommandBuffer = RecordUploadCommands(imageUploadDescs, bufferUploadDesc, stageZone);
        //DEBUGGER_TRACE("rendering thread fun - recording on rendering thread");
        //co_await _oneShotSubmissionHandler->AppendCopyCommandSubmission(uploadCommandBuffer);
        
         
        DEBUGGER_TRACE("rendering thread fun - copy submitted and signaled as done");

        co_await concurrencpp::resume_on(*_poolExecutor); // temporary, release resources on pool thread
        LoadedModel res{};

        DEBUGGER_TRACE("loading done");
        co_return res;
    }

    




} 