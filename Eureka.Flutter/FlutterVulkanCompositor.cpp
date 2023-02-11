#include "FlutterVulkanCompositor.hpp"
#include "../Eureka.Vulkan/PipelinePresets.hpp"
#include <profiling.hpp>
/*
Here are the general steps required to render a textured quad using Vulkan:

Set up the Vulkan environment, including creating an instance, device, and swap chain.
Create a render pass that describes how the output should be rendered to the swap chain's images.
Create a graphics pipeline that defines how the vertices and fragments of the quad are processed. This includes shaders for vertex processing, fragment processing, and any additional stages as necessary.
Allocate and bind memory for the vertex and index buffers that will store the vertices and indices of the quad.
Create a uniform buffer that stores any constant data that needs to be passed to the shaders, such as the model-view-projection matrix.
Load the texture data and create a texture image and a sampler to be used by the fragment shader.
Record a command buffer that specifies the render pass and pipeline to be used, and bind the vertex, index, and uniform buffers.
Submit the command buffer to the queue and present the image to the screen.
Note that this is just a high-level overview of the steps involved in rendering a textured quad using Vulkan. In practice, there are many additional details and optimizations that you will need to consider.


*/
namespace eureka::flutter
{
    static constexpr uint64_t BACKING_STORE_IMAGE_POOL_DEFAULT_SIZE = 5120 * 1440 * 10;

    static void* FlutterGetInstanceProcAddressCallback(void* /*userData*/,
                                                       FlutterVulkanInstanceHandle /*instance*/,
                                                       const char* procname)
    {
        if(std::string_view("vkGetInstanceProcAddr") != procname)
        {
            DEBUGGER_TRACE("bad stuff");
            return nullptr;
        }
        else
        {
            return reinterpret_cast<void*>(vkGetInstanceProcAddr);
        }
    }


    


    FlutterLayersViewPass::FlutterLayersViewPass(graphics::GlobalInheritedData globalInheritedData) :

        graphics::IViewPass(std::move(globalInheritedData))
    {





    }
    void FlutterLayersViewPass::BindToTargetPass(graphics::TargetInheritedData inheritedData)
    {
        _targetInheritedData = std::move(inheritedData);


        vulkan::PipelineLayoutCreationPreset imguiPipelineLayoutPreset(vulkan::PipelinePresetType::eTexturedRegion, *_globalInheritedData.layout_cache);
        _pipelineLayout = std::make_shared<vulkan::PipelineLayout>(_globalInheritedData.device, imguiPipelineLayoutPreset.GetCreateInfo());
        vulkan::PipelineCreationPreset imguiPipelinePreset(
            vulkan::PipelinePresetType::eTexturedRegion, *_globalInheritedData.shader_cache, _pipelineLayout->Get(), _targetInheritedData.render_pass->Get());

        _pipeline = vulkan::Pipeline(_globalInheritedData.device, _pipelineLayout, _targetInheritedData.render_pass, imguiPipelinePreset.GetCreateInfo());


    }
    void FlutterLayersViewPass::RecordDraw(const graphics::RecordParameters& params)
    {
        params.command_buffer.BindGraphicsPipeline(_pipeline.Get());



        VkViewport viewport
        {
            .x = 0.0f,
            .y = 0.0f,
            .width = (float)_w,
            .height = (float)_h,
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };

        params.command_buffer.SetViewport(viewport);

        for(size_t i = 0u; i < _upcomingDrawLayers.size(); ++i)
        {
            auto pLayer = _upcomingDrawLayers[i];

            if(pLayer->type == FlutterLayerContentType::kFlutterLayerContentTypeBackingStore)
            {
                auto backingStore = pLayer->backing_store;
                auto backingStoreData = static_cast<BackingStoreData*>(backingStore->vulkan.user_data);
                params.command_buffer.Bind(
                    VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS,
                    _pipelineLayout->Get(),
                    backingStoreData->descriptor_set.Get(),
                    0u
                );

                auto layerOffset = pLayer->offset;
                auto layerSize = pLayer->size;

                auto widthPortion = layerSize.width / _w;
                auto heightPortion = layerSize.height / _h;

                vulkan::ScaleTranslatePushConstantsBlock pushConstanst
                {
                    .scale = Eigen::Vector2f(widthPortion, heightPortion),
                    .translate = Eigen::Vector2f(layerOffset.x, layerOffset.y) // TODO portion in NDC
                };

                params.command_buffer.PushConstants(_pipelineLayout->Get(), VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, pushConstanst);

                VkRect2D scissorRect;
                scissorRect.offset.x = (int32_t)layerOffset.x;
                scissorRect.offset.y = (int32_t)layerOffset.y;
                scissorRect.extent.width = (uint32_t)(layerSize.width);
                scissorRect.extent.height = (uint32_t)(layerSize.height);


                params.command_buffer.SetScissor(scissorRect);
                params.command_buffer.Draw(
                    6,
                    1,
                    0,
                    0
                );
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////



    FlutterVulkanCompositor::FlutterVulkanCompositor(std::shared_ptr<vulkan::Instance>              instance,
                                                     graphics::GlobalInheritedData                  globalInheritedData,
                                                     std::shared_ptr<vulkan::FrameContext>          frameContext, // TODO should be the other way around
                                                     std::shared_ptr<eureka::graphics::ITargetPass> targetPass) : // TODO should be the other way around

        _instance(std::move(instance)),
        _globalInheritedData(std::move(globalInheritedData)),
        _graphicsQueue(_globalInheritedData.device->GetGraphicsQueue()),
        _frameContext(std::move(frameContext)),
        _targetPass(std::move(targetPass)), // TODO should be the other way around
        _backingStorePool(std::make_shared<vulkan::ImageMemoryPool>(
            _globalInheritedData.resource_allocator,
            BACKING_STORE_IMAGE_POOL_DEFAULT_SIZE,
            VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT,
            vulkan::Image2DAllocationPreset::eR8G8B8A8UnormSampledShaderResourceRenderTargetTransferSrcDst))
    {
        _flutterRendererConfig.type = FlutterRendererType::kVulkan;
        _flutterRendererConfig.vulkan.struct_size = sizeof(FlutterVulkanRendererConfig);
        _flutterRendererConfig.vulkan.version = _instance->ApiVersion().Get();
        _flutterRendererConfig.vulkan.instance = _instance->Get();
        _flutterRendererConfig.vulkan.physical_device = _globalInheritedData.device->GetPhysicalDevice();
        _flutterRendererConfig.vulkan.device = _globalInheritedData.device->GetDevice();
        _flutterRendererConfig.vulkan.queue_family_index = _graphicsQueue.Family();
        _flutterRendererConfig.vulkan.queue = _graphicsQueue.Get();
        _flutterRendererConfig.vulkan.enabled_instance_extension_count = _instance->EnabledExtentions().size();
        _flutterRendererConfig.vulkan.enabled_instance_extensions = _instance->EnabledExtentions().data();
        _flutterRendererConfig.vulkan.enabled_device_extension_count = _globalInheritedData.device->EnabledExtentions().size();
        _flutterRendererConfig.vulkan.enabled_device_extensions = _globalInheritedData.device->EnabledExtentions().data();
        _flutterRendererConfig.vulkan.get_instance_proc_address_callback = FlutterGetInstanceProcAddressCallback;
        _flutterRendererConfig.vulkan.get_next_image_callback = GetNextImageStatic;
        _flutterRendererConfig.vulkan.present_image_callback = PresentImageStatic;

        _flutterCompositor.struct_size = sizeof(FlutterCompositor);
        _flutterCompositor.user_data = this;
        _flutterCompositor.create_backing_store_callback = CreateBackingStoreStatic;
        _flutterCompositor.collect_backing_store_callback = CollectBackingStoreStatic;
        _flutterCompositor.present_layers_callback = LayersPresentStatic;


        _backingStoreSampler = vulkan::CreateSampler(_globalInheritedData.device, vulkan::SamplerCreationPreset::eLinearClampToEdge);

        _layersViewPass = std::make_shared<FlutterLayersViewPass>(_globalInheritedData);

        _targetPass->AddViewPass(_layersViewPass);
    }

    const FlutterRendererConfig& FlutterVulkanCompositor::GetFlutterRendererConfig() const
    {
        return _flutterRendererConfig;
    }

    const FlutterCompositor& FlutterVulkanCompositor::GetFlutterCompositor() const
    {
        return _flutterCompositor;
    }

    bool FlutterVulkanCompositor::CreateBackingStore(const FlutterBackingStoreConfig* config,
                                                     FlutterBackingStore*             backingStoreOut)
    {
        PROFILE_CATEGORIZED_SCOPE("CreateBackingStore", eureka::profiling::Color::Green, eureka::profiling::PROFILING_CATEGORY_RENDERING);
        _renderDoc.StartCapture(_globalInheritedData.device->GetDevice());
        //DEBUGGER_TRACE("CreateBackingStore {} {}", config->size.width, config->size.height);
        VkExtent2D extent {
            .width = static_cast<uint32_t>(config->size.width),
            .height = static_cast<uint32_t>(config->size.height),
        };

        vulkan::Image2DProperties imageProps
        {
            .extent = extent,
            .preset = vulkan::Image2DAllocationPreset::eR8G8B8A8UnormSampledShaderResourceRenderTargetTransferSrcDst,
        };
        //auto allocation = _backingStorePool->AllocateImage(extent);

        BackingStoreData* bkData = new BackingStoreData
        {
            .self = this,
            .image = vulkan::PoolAllocatedImage2D(_globalInheritedData.device, _backingStorePool, imageProps),
            .descriptor_set = vulkan::FreeableDescriptorSet(
                _globalInheritedData.device,
                _globalInheritedData.descriptor_allocator,
                _globalInheritedData.layout_cache->GetLayoutHandle(vulkan::DescriptorSet0PresetType::eSingleTexture)
            ),
        };

        std::array<VkDescriptorImageInfo, 1> imageInfo
        {
            VkDescriptorImageInfo
            {
                .sampler = _backingStoreSampler.Get(),
                .imageView = bkData->image.GetView(),
                .imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            }
        };

        bkData->descriptor_set.SetBindings(
            0,
            VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            imageInfo
        );

        bkData->flutter_image = FlutterVulkanImage {
            .struct_size = sizeof(FlutterVulkanImage),
            .image = reinterpret_cast<FlutterVulkanImageHandle>(bkData->image.Get()),
            .format = VK_FORMAT_R8G8B8A8_UNORM,
        };

        backingStoreOut->struct_size = sizeof(FlutterBackingStore);
        backingStoreOut->user_data = nullptr; // we currently do nothing in collect backing store
        backingStoreOut->type = FlutterBackingStoreType::kFlutterBackingStoreTypeVulkan;
        backingStoreOut->vulkan.struct_size = sizeof(FlutterVulkanBackingStore);
        backingStoreOut->vulkan.image = &bkData->flutter_image;
        backingStoreOut->vulkan.user_data = bkData;
        backingStoreOut->vulkan.destruction_callback = DestroyVulkanBackingStoreStatic;

        return true;
    }

    bool FlutterVulkanCompositor::CollectBackingStore(const FlutterBackingStore* /*backingStore*/)
    {
        //BackingStoreData* bkData = static_cast<BackingStoreData*>(backingStore->user_data);
        //_allocator->DeallocateImage(bkData->allocation);
        //delete bkData;
        DEBUGGER_TRACE("CollectBackingStore");
        _renderDoc.EndCapture(_globalInheritedData.device->GetDevice());
        return true;
    }

    void FlutterVulkanCompositor::DestroyVulkanBackingStore(BackingStoreData* data)
    {
        PROFILE_CATEGORIZED_SCOPE("DestroyVulkanBackingStore", eureka::profiling::Color::Green, eureka::profiling::PROFILING_CATEGORY_RENDERING);
        //DEBUGGER_TRACE("DestroyVulkanBackingStore");
        _frameContext->SyncCurrentFrame(); // TODO, HACKISH to prevent destroting in flight descriptor, maybe pass this to "collect" list that will be released later
        delete data;
    }

    void FlutterVulkanCompositor::EnqueueImageBarriers(
        const FlutterLayer** layers, 
        size_t layersCount,
        vulkan::LinearCommandBufferHandle commandBuffer
        )
    {
        // image memory barrier before setting up the final render pass on which we will read the images
        // https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples#first-draw-writes-to-a-color-attachment-second-draw-samples-from-that-color-image-in-the-fragment-shader

        svec5<VkImageMemoryBarrier> imageBarriers;
        //svec5<VkImageMemoryBarrier2> imageBarriers;

        for (auto i = 0u; i < layersCount; ++i)
        {
            auto pLayer = layers[i];

            if (pLayer->type == FlutterLayerContentType::kFlutterLayerContentTypeBackingStore)
            {

                auto backingStore = pLayer->backing_store;
                auto backingStoreData = static_cast<BackingStoreData*>(backingStore->vulkan.user_data);

                imageBarriers.emplace_back(VkImageMemoryBarrier
                    { 
                        .sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
                        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        .newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
                        .image = backingStoreData->image.Get(),
                        .subresourceRange = VkImageSubresourceRange
                         {
                            .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                            .baseMipLevel = 0,
                            .levelCount = 1,
                            .layerCount = 1
                         },
                    });

                //imageBarriers.emplace_back(VkImageMemoryBarrier2KHR
                //    {
                //       .sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                //       .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, // we rendered to this texture
                //       .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
                //       .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR, // we will read it in the fragment shader
                //       .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT_KHR,
                //       .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                //       .newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
                //       .image = backingStoreData->image.Get(),
                //       .subresourceRange = VkImageSubresourceRange
                //       {
                //          .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                //          .baseMipLevel = 0,
                //          .levelCount = 1,
                //          .layerCount = 1
                //       }
                //    });
            }
        }

        //VkDependencyInfoKHR dependencyInfo
        //{
        //    .sType = VkStructureType::VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        //    .imageMemoryBarrierCount = static_cast<uint32_t>(imageBarriers.size()),
        //    .pImageMemoryBarriers = imageBarriers.data(),
        //};
        vkCmdPipelineBarrier(commandBuffer.Get(),
            VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0u, //VkDependencyFlagBits::
            0u, nullptr, // memory barriers
            0u, nullptr, // buffer memory barriers
            static_cast<uint32_t>(imageBarriers.size()),
            imageBarriers.data()
        );
        //commandBuffer.PipelineBarrierKHR(dependencyInfo);
    }

    bool FlutterVulkanCompositor::PresentLayers(const FlutterLayer** layers, size_t layersCount)
    {
        PROFILE_CATEGORIZED_SCOPE("PresentLayers", eureka::profiling::Color::Green, eureka::profiling::PROFILING_CATEGORY_RENDERING);
        // These should occur outside of this function in any case
        _frameContext->BeginFrame(); // synchronizes
        _targetPass->Prepare();



        auto [valid, targetReady] = _targetPass->PreRecord();


        if(!valid)
        {
            return false;
        }
        auto [mainCommandBuffer, doneSemaphore] = _frameContext->NewGraphicsPresentCommandBuffer();

        mainCommandBuffer.Begin();

        //DEBUGGER_TRACE("PresentLayers {}", layersCount);


        EnqueueImageBarriers(layers, layersCount, mainCommandBuffer);

        dspan<const FlutterLayer*> upcomingLayers(layers, layers + layersCount);

        _layersViewPass->SetUpcomingDrawLayers(upcomingLayers);

        _targetPass->RecordDraw({ mainCommandBuffer }); // will call flutter layers view with associated layers

        mainCommandBuffer.End();

        _targetPass->PostRecord();


        std::array<VkSemaphore, 1> waitSemaphores {targetReady};

        std::array<VkPipelineStageFlags, 1> waitStageMasks {
            VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        auto doneSemaphoreHandle = doneSemaphore.Get();

        auto         mainCommandBufferHandle = mainCommandBuffer.Get();
        VkSubmitInfo submitInfo {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
            .pWaitSemaphores = waitSemaphores.data(),
            .pWaitDstStageMask = waitStageMasks.data(),
            .commandBufferCount = 1,
            .pCommandBuffers = &mainCommandBufferHandle,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &doneSemaphoreHandle,
        };

        _graphicsQueue.Submit(submitInfo, _frameContext->NewGraphicsSubmitFence());

        // On Intel (single queue), we have a race with the presentation engine
        // https://stackoverflow.com/questions/63320119/vksubpassdependency-specification-clarification
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkQueuePresentKHR.html
        // https://stackoverflow.com/questions/68050676/can-vkqueuepresentkhr-be-synced-using-a-pipeline-barrier
        _targetPass->PostSubmit(doneSemaphore);
        //_graphicsQueue->waitIdle();
        _frameContext->EndFrame();

        return true;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    //                                static callbacks boilerplate
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool FlutterVulkanCompositor::CreateBackingStoreStatic(const FlutterBackingStoreConfig* config,
                                                           FlutterBackingStore*             backingStoreOut,
                                                           void*                            userData)
    {
        return static_cast<FlutterVulkanCompositor*>(userData)->CreateBackingStore(config, backingStoreOut);
    }

    bool FlutterVulkanCompositor::CollectBackingStoreStatic(const FlutterBackingStore* backingStore, void* userData)
    {
        return static_cast<FlutterVulkanCompositor*>(userData)->CollectBackingStore(backingStore);
    }

    void FlutterVulkanCompositor::DestroyVulkanBackingStoreStatic(void* userData)
    {
        auto backingStoreData = static_cast<BackingStoreData*>(userData);
        backingStoreData->self->DestroyVulkanBackingStore(backingStoreData);
    }

    bool FlutterVulkanCompositor::LayersPresentStatic(const FlutterLayer** layers, size_t layersCount, void* userData)
    {
        return static_cast<FlutterVulkanCompositor*>(userData)->PresentLayers(layers, layersCount);
    }

    FlutterVulkanImage FlutterVulkanCompositor::GetNextImageStatic(void* /*userData*/,
                                                                   const FlutterFrameInfo* /*frameInfo*/)
    {
        DEBUGGER_TRACE("GetNextImageStatic");
        assert(false);

        return FlutterVulkanImage {};
    }

    bool FlutterVulkanCompositor::PresentImageStatic(void* /*userData*/, const FlutterVulkanImage* /*image*/)
    {
        DEBUGGER_TRACE("PresentImageStatic");
        assert(false);
        return true;
    }



} // namespace eureka::flutter