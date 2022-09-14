#pragma once
#include <imgui.h>
#include <Buffer.hpp>
#include <imgui_internal.h>
#include "Pipeline.hpp"
#include "Pool.hpp"
#include "Image.hpp"
#include "Commands.hpp"

#include "UploadRingBuffer.hpp"
#include "GraphicsDefaults.hpp"
#include "CommandsUtils.hpp"
#include "Pipeline.hpp"
#include "PipelineTypes.hpp"
#include "Window.hpp"
#include "Descriptors.hpp"
#include "IPass.hpp"
#include "AsyncDataLoader.hpp"

namespace eureka
{


    class ImGuiViewPass : public IViewPass
    {
        DeviceContext& _deviceContext;
        std::shared_ptr<AsyncDataLoader>       _asyncDataLoader;
        PoolExecutor                           _poolExecutor;
        std::shared_ptr<MTDescriptorAllocator> _descPool;
        SampledImage2D                         _fontImage;
        VertexAndIndexHostVisibleDeviceBuffer  _vertexIndexBuffer;


        std::shared_ptr<ImGuiPipeline>       _pipeline;
        bool _active{ false };
        uint64_t _vertexBufferOffset{ 0 };
        FreeableDescriptorSet _descriptorSet;
        bool _first{ true };
        future_t<void> Setup(std::shared_ptr<Window> window, std::shared_ptr<PipelineCache> pipelineCache);
    public:
        ImGuiViewPass(
            DeviceContext& deviceContext,
            std::shared_ptr<Window> window,
            std::shared_ptr<PipelineCache> pipelineCache,
            std::shared_ptr<MTDescriptorAllocator> descPool,
            std::shared_ptr<AsyncDataLoader> asyncDataLoader,
            PoolExecutor poolExecutor
        );
        ~ImGuiViewPass();
   
        bool Active() const
        {
            return _active;
        }
        void Layout();
        void SyncBuffers();
        void RecordDrawCommands(vk::CommandBuffer commandBuffer);
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

