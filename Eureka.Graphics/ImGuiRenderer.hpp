#pragma once
#include <imgui.h>
#include <Buffer.hpp>
#include <imgui_internal.h>
#include "Pipeline.hpp"
#include "Pool.hpp"
#include "Image.hpp"
#include "Commands.hpp"
#include "SubmissionThreadExecutionContext.hpp"
#include "OneShotCopySubmission.hpp"
#include "UploadRingBuffer.hpp"
#include "GraphicsDefaults.hpp"
#include "CommandsUtils.hpp"
#include "Pipeline.hpp"
#include "PipelineTypes.hpp"
#include "Window.hpp"

namespace eureka
{
    
    inline constexpr uint64_t EUREKA_MAX_IMGUI_VERTEX_INDEX_BYTES = 1024 * 1024;


    class ImGuiRenderer
    {
        DeviceContext& _deviceContext;
        std::shared_ptr<HostWriteCombinedRingPool> _uploadPool;
        PoolExecutor _poolExecutor;

        std::shared_ptr<OneShotSubmissionHandler> _oneShotSubmissionHandler;
        SampledImage2D _fontImage;
        VertexAndIndexHostVisibleDeviceBuffer _vertexIndexBuffer;


        std::shared_ptr<ImGuiPipeline>       _pipeline;
        bool _active{ false };
        uint64_t _vertexBufferOffset{ 0 };
    public:
        ImGuiRenderer(
            DeviceContext& deviceContext,
            std::shared_ptr<Window> window,
            std::shared_ptr<PipelineCache> pipelineCache,
            std::shared_ptr<MTDescriptorAllocator> descPool,
            std::shared_ptr<OneShotSubmissionHandler> oneShotSubmissionHandler,
            std::shared_ptr<HostWriteCombinedRingPool> uploadPool,
            PoolExecutor poolExecutor
        );
        ~ImGuiRenderer();
        future_t<void> Setup(std::shared_ptr<Window> window, std::shared_ptr<PipelineCache> pipelineCache, std::shared_ptr<MTDescriptorAllocator> descPool);
        bool Active() const
        {
            return _active;
        }
        void Layout();
        void SyncBuffers();
        void RecordDrawCommands(vk::CommandBuffer commandBuffer);
        void HandleResize(uint32_t w, uint32_t h) const;
private:
    FreeableDescriptorSet _descriptorSet;
    };

}

