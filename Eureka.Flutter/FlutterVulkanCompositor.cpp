#include "FlutterVulkanCompositor.hpp"
#include "../Eureka.Vulkan/PipelinePresets.hpp"
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

                vulkan::ScaleTranslatePushConstantsBlock pushConstanst
                {
                    .scale = Eigen::Vector2f(2.0f / layerSize.width, 2.0f / layerSize.height),
                    .translate = Eigen::Vector2f(layerOffset.x, layerOffset.y)
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
        _renderDoc.StartCapture(_globalInheritedData.device->GetDevice());
        DEBUGGER_TRACE("CreateBackingStore {} {}", config->size.width, config->size.height);
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
        DEBUGGER_TRACE("DestroyVulkanBackingStore");

        delete data;
    }

    bool FlutterVulkanCompositor::PresentLayers(const FlutterLayer** layers, size_t layersCount)
    {

        // These should occur outside of this function in any case
        _frameContext->BeginFrame();
        _targetPass->Prepare();



        auto [valid, targetReady] = _targetPass->PreRecord();


        if(!valid)
        {
            return false;
        }
        auto [mainCommandBuffer, doneSemaphore] = _frameContext->NewGraphicsPresentCommandBuffer();

        mainCommandBuffer.Begin();

        DEBUGGER_TRACE("PresentLayers {}", layersCount);


        dspan<const FlutterLayer*> upcomingLayers(layers, layers + layersCount);

        _layersViewPass->SetUpcomingDrawLayers(upcomingLayers);

        _targetPass->RecordDraw({ mainCommandBuffer }); // will call flutter layers view with associated layers


        _targetPass->RecordDraw({mainCommandBuffer});

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