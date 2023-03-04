#pragma once

#include "../Eureka.Vulkan/ResourceAllocator.hpp"
#include "../Eureka.Vulkan/Buffer.hpp"
#include "../Eureka.Vulkan/Image.hpp"
#include "../Eureka.Vulkan/Descriptor.hpp"
#include "../Eureka.Vulkan/Pipeline.hpp"


//#include "UploadRingBuffer.hpp"
//#include "GraphicsDefaults.hpp"
//#include "PipelineTypes.hpp"
#include "Window.hpp"
//#include "Descriptors.hpp"
#include "IPass.hpp"

#include <IImGuiLayout.hpp>

namespace eureka::graphics
{
    class AsyncDataLoader;
    class ImGuiPipeline;
    class PipelineCache;

    class ImGuiViewPass : public IViewPass
    {
        TargetInheritedData                                        _targetInheritedData;
        // vulkan resources:
        std::shared_ptr<vulkan::PipelineLayout>                    _pipelineLayout;
        vulkan::Pipeline                                           _pipeline;
        vulkan::FreeableDescriptorSet                              _descriptorSet;
        vulkan::AllocatedImage2D                                            _fontImage;
        vulkan::Sampler                                            _fontSampler;
        vulkan::HostVisibleVertexAndIndexTransferableDeviceBuffer  _vertexIndexBuffer;


        //std::shared_ptr<ImGuiPipeline>       _pipeline;
        bool _active{ false };
        bool _validSize{ false };
        bool _initialized{ false };
        uint64_t _vertexBufferOffset{ 0 };

        future_t<void> Setup();

        std::shared_ptr<IImGuiLayout> _layout;
    public:
        ImGuiViewPass(
            GlobalInheritedData globalInheritedData,
            std::shared_ptr<IImGuiLayout> layout
        );
        ~ImGuiViewPass();
   
        bool Active() const
        {
            return _active;
        }
        void Layout();
        void SyncBuffers();
        void RecordDrawCommands(vulkan::LinearCommandBufferHandle commandBuffer);
        void BindToTargetPass(TargetInheritedData inheritedData) override;
        void HandleResize(uint32_t w, uint32_t h) override;

        void Prepare() override
        {
            Layout();
            SyncBuffers();
        }

        void RecordDraw(const RecordParameters& params) override
        {
            RecordDrawCommands(params.command_buffer);
        }

    };

}

