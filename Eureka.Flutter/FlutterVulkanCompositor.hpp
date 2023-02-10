#pragma once

#include "../Eureka.Graphics/TargetPass.hpp"
#include "../Eureka.Vulkan/FrameContext.hpp"
#include "../Eureka.Vulkan/ImageMemoryPool.hpp"
#include "../Eureka.Vulkan/SwapChain.hpp"
#include <Eureka.Vulkan/Descriptor.hpp>
#include <Eureka.Vulkan/Pipeline.hpp>
#include <RenderDocIntegration.hpp> // TODO remove
#include <flutter/flutter_embedder.h>

namespace eureka::flutter
{

    class FlutterVulkanCompositor
    {
        std::shared_ptr<vulkan::Instance>      _instance;
        graphics::GlobalInheritedData          _globalInheritedData;
        vulkan::Queue                          _graphicsQueue;
        FlutterRendererConfig                  _flutterRendererConfig {};
        FlutterCompositor                      _flutterCompositor;
        std::shared_ptr<vulkan::FrameContext>  _frameContext;
        std::shared_ptr<graphics::ITargetPass> _targetPass;

        std::shared_ptr<vulkan::PipelineLayout> _pipelineLayout;
        vulkan::Pipeline                        _pipeline;
        vulkan::FreeableDescriptorSet           _descriptorSet;
        vulkan::Sampler                         _backingStoreSampler;
        RenderDocIntegration                    _renderDoc;
        std::shared_ptr<vulkan::ImageMemoryPool>                 _backingStorePool;

    public:
        FlutterVulkanCompositor(
            std::shared_ptr<vulkan::Instance>              instance,
            graphics::GlobalInheritedData                  globalInheritedData,
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
            FlutterVulkanCompositor*      self;
            vulkan::PoolAllocatedImage2D  image;
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