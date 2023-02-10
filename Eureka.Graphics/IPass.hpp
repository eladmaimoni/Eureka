#pragma once
#include <Eureka.Vulkan/Commands.hpp>
#include <Eureka.Vulkan/RenderPass.hpp>
#include <Eureka.Vulkan/ResourceAllocator.hpp>
#include <Eureka.Vulkan/Synchronization.hpp>
#include <Eureka.Vulkan/DescriptorAllocators.hpp>
#include <Eureka.Vulkan/ShadersCache.hpp>
#include <Eureka.Vulkan/DescriptorLayoutCache.hpp>

#include "AsyncDataLoader.hpp"

namespace eureka::graphics
{

    struct GlobalInheritedData
    {
        // TODO add memory pools
        std::shared_ptr<vulkan::Device>                         device;
        std::shared_ptr<vulkan::ResourceAllocator>              resource_allocator;
        std::shared_ptr<vulkan::ShaderCache>                    shader_cache;
        std::shared_ptr<vulkan::DescriptorSetLayoutCache>       layout_cache;
        std::shared_ptr<vulkan::FreeableDescriptorSetAllocator> descriptor_allocator;
        std::shared_ptr<AsyncDataLoader>                        async_data_loader;
    };

    class RenderTarget;

    struct RecordParameters
    {
        vulkan::LinearCommandBufferHandle command_buffer;
    };

    class IPass
    {
    protected:
        GlobalInheritedData                 _globalInheritedData;
        IPass(GlobalInheritedData globalInheritedData) : _globalInheritedData(std::move(globalInheritedData)) {}
    public:
        virtual void Prepare() = 0;
        //virtual void Begin() = 0;
        virtual void RecordDraw(const RecordParameters& params) = 0;
        //virtual void End() = 0;
    };

    struct TargetPassBeginInfo
    {
        bool        valid;
        VkSemaphore target_available_wait_semaphore;
    };


    // data that is passed by data to its descendants (view, meshes)
    // TODO
    struct TargetInheritedData
    {
        std::shared_ptr<vulkan::RenderPass> render_pass;
    };

    class IViewPass : public IPass
    {
        /*
         groups the set of mesh passes passes that uses the same view characteristics
         - per view data (desc set 0)
           in the simple case, holds
           std::vector<IMeshPass> meshPasses;
           and render them
        */
    public:
        IViewPass(GlobalInheritedData globalInheritedData) : IPass(std::move(globalInheritedData)) {}
        virtual void BindToTargetPass(TargetInheritedData targetInheritedData) = 0;
        virtual void HandleResize(uint32_t w, uint32_t h) = 0;
    };



    class ITargetPass : public IPass
    {
    protected:
        sigslot::signal<uint32_t, uint32_t> _resizeSignal;
        sigslot::scoped_connection          _resizeConnection;
        TargetInheritedData                 _targetInheritedData;
        ITargetPass(GlobalInheritedData globalInheritedData) : IPass(std::move(globalInheritedData)) {}
    public:
        virtual VkExtent2D GetSize() = 0;
        virtual void AddViewPass(std::shared_ptr<IViewPass> viewPass) = 0;
        virtual TargetPassBeginInfo PreRecord() = 0;
        virtual void                PostRecord() = 0;
        virtual void                PostSubmit(vulkan::BinarySemaphoreHandle waitSemaphore) = 0;

        /*
        groups all rendering that is submitted to a specific render target
        maps to a vk::RenderPass instance or a subpass withing it
        an ITargetPass may hold extra subpasses

        in the simple case, holds
        std::vector<IViewPass> viewPasses;
        and render them
        */
        template <typename Callable>
        sigslot::connection ConnectResizeSlot(Callable&& slot)
        {
            return _resizeSignal.connect(std::forward<Callable>(slot));
        }

        const TargetInheritedData& GetTargetInheritedData() const // TODO remove?
        {
            return _targetInheritedData;
        }
    };


    //class IMeshPass : public IPass
    //{
    //public:
    //    virtual vk::PipelineLayout GetPipelineLayout() = 0;
    //    /*
    //    groups a set of objects to be rendered with the same shader / material, to the same target / view
    //    */
    //    //VkPipelineLayout layout;
    //    //std::array<VkDescriptorSetLayout, 4> setLayouts;
    //    //VkPipeline pipeline; // shared

    //    //
    //    // bind descriptor sets that are shared by all objects
    //    // bind pipeline, optionally group objects by the same vertex buffer
    //    //

    //    //
    //    // draw each object with the same effect
    //    // for (obj : object)
    //    //     Bind(object) // material descriptor sets, push constants, vertex data & offsets
    //    //     Draw(object)
    //};

} // namespace eureka
