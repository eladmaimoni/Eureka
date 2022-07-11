#include "RenderingSystem.hpp"
#include "SwapChain.hpp"
#include "RenderTarget.hpp"
#include "GraphicsDefaults.hpp"
#include <profiling_macros.hpp>


namespace eureka
{

    RenderingSystem::RenderingSystem(
        DeviceContext& deviceContext,
        std::shared_ptr<SwapChainDepthColorFrame> swapChainFrame,
        std::shared_ptr<PipelineCache> pipelineCache,
        std::shared_ptr<ImGuiRenderer> imguiRenderer,
        std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext,
        std::shared_ptr<OneShotCopySubmissionHandler>     oneShotCopySubmissionHandler,
        std::shared_ptr<MTDescriptorAllocator>                   descPool,
        Queue graphicsQueue,
        Queue copyQueue
    )
        :
        _deviceContext(deviceContext),
        _swapChainFrame(std::move(swapChainFrame)),
        _pipelineCache(std::move(pipelineCache)),
        _descPool(std::move(descPool)),
        _imguiRenderer(std::move(imguiRenderer)),
        _submissionThreadExecutionContext(/*std::move(*/submissionThreadExecutionContext/*)*/), // TODO
        _oneShotCopySubmissionHandler(std::move(oneShotCopySubmissionHandler)),
        _camera(deviceContext, /*std::move(*/submissionThreadExecutionContext/*)*/),  // TODO
        _graphicsQueue(graphicsQueue),
        _copyQueue(copyQueue)
    {

    }

    RenderingSystem::~RenderingSystem()
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //
    //                        Initialize
    //
    //////////////////////////////////////////////////////////////////////////

    void RenderingSystem::Initialize()
    {
        _submissionThreadExecutionContext->SetCurrentThreadAsRenderingThread();


        _stageZone = HostWriteCombinedBuffer(
            _deviceContext.Allocator(),
            BufferConfig{ .byte_size = sizeof(mesh::COLORED_TRIANGLE_INDEX_DATA) + sizeof(mesh::COLORED_TRIANGLE_VERTEX_DATA) }
        );



        _triangle = VertexAndIndexTransferableDeviceBuffer(
            _deviceContext.Allocator(),
            BufferConfig{ .byte_size = sizeof(mesh::COLORED_TRIANGLE_INDEX_DATA) + sizeof(mesh::COLORED_TRIANGLE_VERTEX_DATA) }
        );


        _lastFrameTime = std::chrono::high_resolution_clock::now();

        _coloredVertexPipeline = _pipelineCache->GetColoredVertexMeshPipeline();


        _camera.SetPosition(Eigen::Vector3f(0.0f, 0.0f, 2.5f));
        _camera.SetLookDirection(Eigen::Vector3f(0.0f, 0.0f, -1.0f));
        _camera.SetVerticalFov(3.14f / 4.0f);

        _constantBufferSet = _descPool->AllocateSet(_coloredVertexPipeline->GetPerViewLayout());

        auto [descType, descInfo] = _camera.DescriptorInfo();

        _constantBufferSet.SetBinding(0, descType, descInfo);


        auto oneShotCopyTriangleCommandBuffer = _submissionThreadExecutionContext->OneShotCopySubmitCommandPool().AllocatePrimaryCommandBuffer();


        _stageZone.Assign(std::span(mesh::COLORED_TRIANGLE_INDEX_DATA), 0);
        _stageZone.Assign(std::span(mesh::COLORED_TRIANGLE_VERTEX_DATA), sizeof(mesh::COLORED_TRIANGLE_INDEX_DATA));
        {
            ScopedCommands commands(oneShotCopyTriangleCommandBuffer);

            oneShotCopyTriangleCommandBuffer.copyBuffer(
                _stageZone.Buffer(),
                _triangle.Buffer(),
                { vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = _triangle.ByteSize()} }
            );
        }

        _oneShotCopySubmissionHandler->AppendOneShotCommandBufferSubmission(std::move(oneShotCopyTriangleCommandBuffer));

        _resizeConnection = _swapChainFrame->ConnectResizeSlot(
            [this](uint32_t w, uint32_t h)
        {
            HandleResize(w, h);
        });
        auto renderArea = _swapChainFrame->RenderArea();
        HandleResize(renderArea.extent.width, renderArea.extent.height);
    }

    void RenderingSystem::HandleResize(uint32_t w, uint32_t h)
    {
        DEBUGGER_TRACE("handle swap chain resize");

   
        _camera.SetFullViewport(0, 0, w, h);
   
        _imguiRenderer->HandleResize(w, h);



        RunOne(); // refresh before next resize
    }

    void RenderingSystem::Deinitialize()
    {
        _deviceContext.LogicalDevice()->waitIdle();
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //                        RunOne
    //
    //////////////////////////////////////////////////////////////////////////

    static constexpr std::size_t MAX_COPY_SUBMITS_PER_FRAME = 10;


    void RenderingSystem::RunOne()
    {
        PROFILE_CATEGORIZED_SCOPE("RunOne", Profiling::Color::Blue, Profiling::PROFILING_CATEGORY_RENDERING);
        _submissionThreadExecutionContext->Executor().loop_all(MAX_COPY_SUBMITS_PER_FRAME);

        _oneShotCopySubmissionHandler->PollPendingOneShotSubmissions();
        _oneShotCopySubmissionHandler->PollDoneOneShotSubmissions();
    
        _imguiRenderer->Layout();

        _imguiRenderer->SyncBuffers();


        auto [commandBuffer, frameAvailableWaitSemaphore] = _swapChainFrame->BeginFrameRecording();

        //PROFILE_CATEGORIZED_SCOPE("Record Submit", Profiling::Color::DarkGray, Profiling::PROFILING_CATEGORY_RENDERING);

        RecordMainRenderPass(commandBuffer);

        std::array<vk::Semaphore, 1> waitSemaphores
        {
            frameAvailableWaitSemaphore
        };

        std::array<vk::PipelineStageFlags, 1> waitStageMasks
        {
            vk::PipelineStageFlagBits::eColorAttachmentOutput
        };
        _swapChainFrame->EndFrameRecordingAndSubmit(waitSemaphores, waitStageMasks);
        _swapChainFrame->Present();
    }



    void RenderingSystem::RecordMainRenderPass(vk::CommandBuffer renderingCommandBuffer)
    {
        _swapChainFrame->BeginPrimaryRenderPass();

        renderingCommandBuffer.setViewport(0, { _camera.Viewport() });
        renderingCommandBuffer.setScissor(0, { _swapChainFrame->RenderArea() }); // TODO from camera, 

        renderingCommandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            _coloredVertexPipeline->Layout(),
            0,
            { _constantBufferSet.Get() },
            nullptr
        );


        renderingCommandBuffer.bindPipeline(
            vk::PipelineBindPoint::eGraphics,
            _coloredVertexPipeline->Get()
        );
        
        renderingCommandBuffer.bindIndexBuffer(
            _triangle.Buffer(),
            0,
            vk::IndexType::eUint32
        );

        renderingCommandBuffer.bindVertexBuffers(
            0,
            { _triangle.Buffer() },
            { sizeof(mesh::COLORED_TRIANGLE_INDEX_DATA) }
        );
 

        renderingCommandBuffer.drawIndexed(3, 1, 0, 0, 1);


        _swapChainFrame->EndPrimaryRenderPass();

    }

}