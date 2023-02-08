#include "FlutterVulkanCompositor.hpp"
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

    FlutterVulkanCompositor::FlutterVulkanCompositor(std::shared_ptr<vulkan::Instance>              instance,
                                                     std::shared_ptr<vulkan::Device>                device,
                                                     std::shared_ptr<vulkan::ResourceAllocator>     allocator,
                                                     std::shared_ptr<vulkan::FrameContext>          frameContext,
                                                     std::shared_ptr<eureka::graphics::ITargetPass> targetPass
    ) :
        _instance(std::move(instance)),
        _device(std::move(device)),
        _allocator(std::move(allocator)),
        _graphicsQueue(_device->GetGraphicsQueue()),
        _frameContext(std::move(frameContext)),
        _targetPass(std::move(targetPass)),
        _backingStorePool(_allocator, BACKING_STORE_IMAGE_POOL_DEFAULT_SIZE, VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT, vulkan::Image2DAllocationPreset::eR8G8B8A8UnormSampledShaderResourceRenderTargetTransferSrcDst)
    {
        _flutterRendererConfig.type = FlutterRendererType::kVulkan;
        _flutterRendererConfig.vulkan.struct_size = sizeof(FlutterVulkanRendererConfig);
        _flutterRendererConfig.vulkan.version = _instance->ApiVersion().Get();
        _flutterRendererConfig.vulkan.instance = _instance->Get();
        _flutterRendererConfig.vulkan.physical_device = _device->GetPhysicalDevice();
        _flutterRendererConfig.vulkan.device = _device->GetDevice();
        _flutterRendererConfig.vulkan.queue_family_index = _graphicsQueue.Family();
        _flutterRendererConfig.vulkan.queue = _graphicsQueue.Get();
        _flutterRendererConfig.vulkan.enabled_instance_extension_count = _instance->EnabledExtentions().size();
        _flutterRendererConfig.vulkan.enabled_instance_extensions = _instance->EnabledExtentions().data();
        _flutterRendererConfig.vulkan.enabled_device_extension_count = _device->EnabledExtentions().size();
        _flutterRendererConfig.vulkan.enabled_device_extensions = _device->EnabledExtentions().data();
        _flutterRendererConfig.vulkan.get_instance_proc_address_callback = FlutterGetInstanceProcAddressCallback;
        _flutterRendererConfig.vulkan.get_next_image_callback = GetNextImageStatic;
        _flutterRendererConfig.vulkan.present_image_callback = PresentImageStatic;

        _flutterCompositor.struct_size = sizeof(FlutterCompositor);
        _flutterCompositor.user_data = this;
        _flutterCompositor.create_backing_store_callback = CreateBackingStoreStatic;
        _flutterCompositor.collect_backing_store_callback = CollectBackingStoreStatic;
        _flutterCompositor.present_layers_callback = LayersPresentStatic;
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
        _renderDoc.StartCapture(_device->GetDevice());
        DEBUGGER_TRACE("CreateBackingStore {} {}", config->size.width, config->size.height);
        VkExtent2D extent {
            .width = static_cast<uint32_t>(config->size.width),
            .height = static_cast<uint32_t>(config->size.height),
        };
        
        auto allocation = _backingStorePool.AllocateImage(extent);

        BackingStoreData* bkData = new BackingStoreData;
        bkData->self = this;
        bkData->allocation = allocation;
        bkData->flutter_image = FlutterVulkanImage {
            .struct_size = sizeof(FlutterVulkanImage),
            .image = reinterpret_cast<FlutterVulkanImageHandle>(allocation.image),
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
        _renderDoc.EndCapture(_device->GetDevice());
        return true;
    }

    void FlutterVulkanCompositor::DestroyVulkanBackingStore(BackingStoreData* data)
    {
        DEBUGGER_TRACE("DestroyVulkanBackingStore");
        _backingStorePool.DeallocateImage(data->allocation);
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
        for(size_t i = 0u; i < layersCount; ++i)
        {
            auto pLayer = layers[i];

            if(pLayer->type == FlutterLayerContentType::kFlutterLayerContentTypeBackingStore)
            {}
        }

        _targetPass->RecordDraw({mainCommandBuffer});
        _targetPass->PostRecord();

        mainCommandBuffer.End();

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