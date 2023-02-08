#pragma once

#include "../Eureka.Graphics/TargetPass.hpp"
#include "../Eureka.Vulkan/FrameContext.hpp"
#include "../Eureka.Vulkan/ImageMemoryPool.hpp"
#include "../Eureka.Vulkan/SwapChain.hpp"
#include <RenderDocIntegration.hpp> // TODO remove
#include <flutter/flutter_embedder.h>

namespace eureka::flutter
{

    class FlutterVulkanCompositor
    {
        std::shared_ptr<vulkan::Instance>          _instance;
        std::shared_ptr<vulkan::Device>            _device;
        std::shared_ptr<vulkan::ResourceAllocator> _allocator;
        vulkan::Queue                              _graphicsQueue;
        FlutterRendererConfig                      _flutterRendererConfig {};
        FlutterCompositor                          _flutterCompositor;
        std::shared_ptr<vulkan::FrameContext>      _frameContext;
        std::shared_ptr<graphics::ITargetPass>     _targetPass;

        RenderDocIntegration    _renderDoc;
        vulkan::ImageMemoryPool _backingStorePool;

    public:
        FlutterVulkanCompositor(
            std::shared_ptr<vulkan::Instance>              instance,
            std::shared_ptr<vulkan::Device>                device,
            std::shared_ptr<vulkan::ResourceAllocator>     allocator,
            std::shared_ptr<vulkan::FrameContext>          frameContext, // TODO should probably not be here?
            std::shared_ptr<eureka::graphics::ITargetPass> targetPass // TODO should probably not be here?
        );

        //
        // accessors for embedder
        //
        const FlutterRendererConfig& GetFlutterRendererConfig() const;
        const FlutterCompositor&     GetFlutterCompositor() const;

    private:
        //
        // Flutter callbacks (members)
        //
        struct BackingStoreData
        {
            FlutterVulkanCompositor* self;
            vulkan::ImageAllocation  allocation;
            FlutterVulkanImage       flutter_image;
        };

        bool CreateBackingStore(const FlutterBackingStoreConfig* config, FlutterBackingStore* backingStoreOut);
        bool CollectBackingStore(const FlutterBackingStore* backingStore);
        void DestroyVulkanBackingStore(BackingStoreData* data);
        bool PresentLayers(const FlutterLayer** layers, size_t layersCount);

        //
        // Flutter callbacks boilerplate (static)
        //
        static bool CreateBackingStoreStatic(const FlutterBackingStoreConfig* config,
                                             FlutterBackingStore*             backingStoreOut,
                                             void*                            userData);

        static bool               CollectBackingStoreStatic(const FlutterBackingStore* backingStore, void* userData);
        static void               DestroyVulkanBackingStoreStatic(void* userData);
        static bool               LayersPresentStatic(const FlutterLayer** layers, size_t layersCount, void* userData);
        static FlutterVulkanImage GetNextImageStatic(void* user_data, const FlutterFrameInfo* frame_info);
        static bool               PresentImageStatic(void* user_data, const FlutterVulkanImage* image);
    };

} // namespace eureka::flutter