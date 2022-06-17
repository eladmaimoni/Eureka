#pragma once

namespace eureka
{
    class RenderTarget;

    struct RecordParameters
    {
        vk::CommandBuffer command_buffer;
    };

    class IPass
    {
    public:
        virtual void Prepare() = 0;
        //virtual void Begin() = 0;
        virtual void RecordDraw(const RecordParameters& params) = 0;
        //virtual void End() = 0;
    };


    struct TargetPassBeginInfo
    {
        bool           valid;
        vk::Semaphore  target_available_wait_semaphore;
    };

    class ITargetPass : public IPass
    {
    protected:
        sigslot::signal<uint32_t, uint32_t>      _resizeSignal;
        sigslot::scoped_connection               _resizeConnection;
    public:
        virtual vk::Extent2D GetSize() = 0;

        virtual TargetPassBeginInfo PreRecord() = 0;
        virtual void PostRecord() = 0;
        virtual void PostSubmit(vk::Semaphore waitSemaphore) = 0;
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
        virtual void HandleResize(uint32_t w, uint32_t h) = 0;
    };


    class IMeshPass : public IPass
    {
    public:
        virtual vk::PipelineLayout GetPipelineLayout() = 0;
        /*
        groups a set of objects to be rendered with the same shader / material, to the same target / view
        */
        //VkPipelineLayout layout;
        //std::array<VkDescriptorSetLayout, 4> setLayouts;
        //VkPipeline pipeline; // shared

        // 
        // bind descriptor sets that are shared by all objects
        // bind pipeline, optionally group objects by the same vertex buffer
        //

        // 
        // draw each object with the same effect
        // for (obj : object)
        //     Bind(object) // material descriptor sets, push constants, vertex data & offsets
        //     Draw(object)
    };

}

