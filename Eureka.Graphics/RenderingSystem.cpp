#include "RenderingSystem.hpp"
#include "SwapChain.hpp"
#include "RenderTarget.hpp"
#include "GraphicsDefaults.hpp"
#include <profiling_macros.hpp>


namespace eureka
{

    RenderingSystem::RenderingSystem(
        DeviceContext& deviceContext,
        std::shared_ptr<SwapChainFrameContext> frameContext,
        std::shared_ptr<PipelineCache> pipelineCache,
        std::shared_ptr<ImGuiRenderer> imguiRenderer,
        std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext,
        std::shared_ptr<OneShotSubmissionHandler>     oneShotSubmissionHandler,
        std::shared_ptr<MTDescriptorAllocator>            descPool,
        Queue graphicsQueue,
        Queue copyQueue
    )
        :
        _deviceContext(deviceContext),
        _frameContext(std::move(frameContext)),
        _pipelineCache(std::move(pipelineCache)),
        _descPool(std::move(descPool)),
        _imguiRenderer(std::move(imguiRenderer)),
        _submissionThreadExecutionContext(/*std::move(*/submissionThreadExecutionContext/*)*/), // TODO
        _oneShotSubmissionHandler(std::move(oneShotSubmissionHandler)),
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

    future_t<void> RenderingSystem::Initialize()
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



        // TODO REMOVE
        _stageZone.Assign(std::span(mesh::COLORED_TRIANGLE_INDEX_DATA), 0);
        _stageZone.Assign(std::span(mesh::COLORED_TRIANGLE_VERTEX_DATA), sizeof(mesh::COLORED_TRIANGLE_INDEX_DATA));




       

        _resizeConnection = _frameContext->ConnectResizeSlot(
            [this](uint32_t w, uint32_t h)
        {
            HandleResize(w, h);
        });
        auto renderArea = _frameContext->RenderArea();
        HandleResize(renderArea.extent.width, renderArea.extent.height);


        co_await _oneShotSubmissionHandler->ResumeOnRecordingContext();
     
        auto [uploadCommandBuffer, uploadDoneSemaphore] = _oneShotSubmissionHandler->NewOneShotCopyCommandBuffer();
        {
            ScopedCommands commands(uploadCommandBuffer);

            uploadCommandBuffer.copyBuffer(
                _stageZone.Buffer(),
                _triangle.Buffer(),
                { vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = _triangle.ByteSize()} }
            );
        }
        _oneShotSubmissionHandler->AppendCopyCommandSubmission(uploadCommandBuffer);
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
     
        try
        {
            PROFILE_CATEGORIZED_SCOPE("RunOne", Profiling::Color::Blue, Profiling::PROFILING_CATEGORY_RENDERING);

            //DEBUGGER_TRACE("RUN ONE START");

            auto beginFrameInfo = _frameContext->BeginFrame();
         
            if (!beginFrameInfo.frame_valid)
            {
                return;
            }
        
            _imguiRenderer->Layout();

            _submissionThreadExecutionContext->Executor().loop_all(MAX_COPY_SUBMITS_PER_FRAME);

            _oneShotSubmissionHandler->SubmitPendingCopies();
            _oneShotSubmissionHandler->PollCopyCompletions();
            _oneShotSubmissionHandler->SubmitPendingGraphics();
            _oneShotSubmissionHandler->PollGraphicsCompletions();

            _imguiRenderer->SyncBuffers();

            _submissionThreadExecutionContext->PreRenderExecutor().loop(100);

            auto [mainCommandBuffer,doneSemaphore] = _frameContext->NewGraphicsCommandBuffer();
        
            //PROFILE_CATEGORIZED_SCOPE("Record Submit", Profiling::Color::DarkGray, Profiling::PROFILING_CATEGORY_RENDERING);

            mainCommandBuffer.begin(vk::CommandBufferBeginInfo());

            RecordMainRenderPass(mainCommandBuffer);

            mainCommandBuffer.end();

            std::array<vk::Semaphore, 1> waitSemaphores
            {
                beginFrameInfo.frame_available_wait_semaphore
            };
         
            std::array<vk::PipelineStageFlags, 1> waitStageMasks
            {
                vk::PipelineStageFlagBits::eColorAttachmentOutput
            };

            vk::SubmitInfo submitInfo
            {
                .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
                .pWaitSemaphores = waitSemaphores.data(),
                .pWaitDstStageMask = waitStageMasks.data(),
                .commandBufferCount = 1,
                .pCommandBuffers = &mainCommandBuffer,
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = &doneSemaphore
            };
            
            _graphicsQueue->submit({ submitInfo }, _frameContext->NewGraphicsSubmitFence());


            _frameContext->EndFrame(doneSemaphore);
        }
        catch (const std::exception& err)
        {
            DEBUGGER_TRACE("RUN ONE error {}", err.what());
        }
        //DEBUGGER_TRACE("RUN ONE END");
    }



    void RenderingSystem::RecordMainRenderPass(vk::CommandBuffer renderingCommandBuffer)
    {
        renderingCommandBuffer.beginRenderPass(_frameContext->PrimaryRenderPassBeginInfo(), vk::SubpassContents::eInline);
       

        renderingCommandBuffer.setViewport(0, { _camera.Viewport() });
        renderingCommandBuffer.setScissor(0, { _frameContext->RenderArea() }); // TODO from camera, 

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

        _imguiRenderer->RecordDrawCommands(renderingCommandBuffer);

        renderingCommandBuffer.endRenderPass();

    }

}