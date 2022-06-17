#include "RenderingSystem.hpp"
#include "SwapChain.hpp"
#include "RenderTarget.hpp"
#include "GraphicsDefaults.hpp"
#include <profiling_macros.hpp>
#include "RenderDocIntegration.hpp"

namespace eureka
{

    RenderingSystem::RenderingSystem(
        DeviceContext& deviceContext,
        Queue graphicsQueue,
        Queue copyQueue,
        std::shared_ptr<FrameContext> frameContext,
        std::shared_ptr<ITargetPass> mainPass,
        std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext,
        std::shared_ptr<OneShotSubmissionHandler> oneShotSubmissionHandler
    )
        :
        _deviceContext(deviceContext),
        _frameContext(std::move(frameContext)),
        _mainPass(std::move(mainPass)),
        _submissionThreadExecutionContext(/*std::move(*/submissionThreadExecutionContext/*)*/), // TODO
        _oneShotSubmissionHandler(std::move(oneShotSubmissionHandler)),
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


        //_stageZone = HostWriteCombinedBuffer(
        //    _deviceContext.Allocator(),
        //    BufferConfig{ .byte_size = sizeof(mesh::COLORED_TRIANGLE_INDEX_DATA) + sizeof(mesh::COLORED_TRIANGLE_VERTEX_DATA) }
        //);



        //_triangle = VertexAndIndexTransferableDeviceBuffer(
        //    _deviceContext.Allocator(),
        //    BufferConfig{ .byte_size = sizeof(mesh::COLORED_TRIANGLE_INDEX_DATA) + sizeof(mesh::COLORED_TRIANGLE_VERTEX_DATA) }
        //);


        //_lastFrameTime = std::chrono::high_resolution_clock::now();

        //_coloredVertexPipeline = _pipelineCache->GetColoredVertexMeshPipeline();


        //_camera.SetPosition(Eigen::Vector3f(0.0f, 0.0f, 2.5f));
        //_camera.SetLookDirection(Eigen::Vector3f(0.0f, 0.0f, -1.0f));
        //_camera.SetVerticalFov(3.14f / 4.0f);

        //_constantBufferSet = _descPool->AllocateSet(_coloredVertexPipeline->GetPerViewLayout());

        //auto [descType, descInfo] = _camera.GetNode()->DescriptorInfo();

        //_constantBufferSet.SetBinding(0, descType, descInfo);



        //// TODO REMOVE
        //_stageZone.Assign(std::span(mesh::COLORED_TRIANGLE_INDEX_DATA), 0);
        //_stageZone.Assign(std::span(mesh::COLORED_TRIANGLE_VERTEX_DATA), sizeof(mesh::COLORED_TRIANGLE_INDEX_DATA));

        _resizeConnection = _mainPass->ConnectResizeSlot(
            [this](uint32_t w, uint32_t h)
        {
            HandleResize(w, h);
        });
        auto [width, height] = _mainPass->GetSize();
        HandleResize(width, height);


        //co_await _oneShotSubmissionHandler->ResumeOnRecordingContext();
     
        //auto [uploadCommandBuffer, uploadDoneSemaphore] = _oneShotSubmissionHandler->NewOneShotCopyCommandBuffer();
        //{
        //    ScopedCommands commands(uploadCommandBuffer);

        //    uploadCommandBuffer.copyBuffer(
        //        _stageZone.Buffer(),
        //        _triangle.Buffer(),
        //        { vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = _triangle.ByteSize()} }
        //    );
        //}
        //_oneShotSubmissionHandler->AppendCopyCommandSubmission(uploadCommandBuffer);
    }

    void RenderingSystem::HandleResize(uint32_t /*w*/, uint32_t /*h*/)
    {
        DEBUGGER_TRACE("handle swap chain resize");

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
            //if (HACK > 0 && HACK < 3)
            //{
            //    RenderDockIntegrationInstance->StartCapture();
            //}
        
            PROFILE_CATEGORIZED_SCOPE("RunOne", Profiling::Color::Blue, Profiling::PROFILING_CATEGORY_RENDERING);

            //DEBUGGER_TRACE("RUN ONE START");

            _frameContext->BeginFrame();

            _submissionThreadExecutionContext->Executor().loop_all(MAX_COPY_SUBMITS_PER_FRAME);

            _oneShotSubmissionHandler->SubmitPendingCopies();
            _oneShotSubmissionHandler->PollCopyCompletions();
            _oneShotSubmissionHandler->SubmitPendingGraphics();
            _oneShotSubmissionHandler->PollGraphicsCompletions();

            _mainPass->Prepare();


            _submissionThreadExecutionContext->PreRenderExecutor().loop(100);

            auto [mainCommandBuffer,doneSemaphore] = _frameContext->NewGraphicsCommandBuffer();

            auto [valid, targetReady] = _mainPass->PreRecord();
            if (!valid)
            {
                return;
            }

            mainCommandBuffer.begin(vk::CommandBufferBeginInfo());
            _mainPass->RecordDraw({ mainCommandBuffer });
            _mainPass->PostRecord();

            mainCommandBuffer.end();

            std::array<vk::Semaphore, 1> waitSemaphores
            {
                targetReady
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
            _mainPass->PostSubmit(doneSemaphore);
            _frameContext->EndFrame();    
        }
        catch (const std::exception& err)
        {
            DEBUGGER_TRACE("RUN ONE error {}", err.what());
        }
        //DEBUGGER_TRACE("RUN ONE END");
    }



    //void RenderingSystem::RecordMainRenderPass(vk::CommandBuffer renderingCommandBuffer)
    //{
        //renderingCommandBuffer.beginRenderPass(_frameContext->PrimaryRenderPassBeginInfo(), vk::SubpassContents::eInline);
       
        //_camera.GetNode()->SyncTransforms();
        //renderingCommandBuffer.setViewport(0, { _camera.GetNode()->Viewport() });
        //renderingCommandBuffer.setScissor(0, { _frameContext->RenderArea() }); // TODO from camera, 

        //renderingCommandBuffer.bindDescriptorSets(
        //    vk::PipelineBindPoint::eGraphics,
        //    _coloredVertexPipeline->Layout(),
        //    0,
        //    { _constantBufferSet.Get() },
        //    nullptr
        //);


        //renderingCommandBuffer.bindPipeline(
        //    vk::PipelineBindPoint::eGraphics,
        //    _coloredVertexPipeline->Get()
        //);
        //
        //renderingCommandBuffer.bindIndexBuffer(
        //    _triangle.Buffer(),
        //    0,
        //    vk::IndexType::eUint32
        //);

        //renderingCommandBuffer.bindVertexBuffers(
        //    0,
        //    { _triangle.Buffer() },
        //    { sizeof(mesh::COLORED_TRIANGLE_INDEX_DATA) }
        //);
 

        //renderingCommandBuffer.drawIndexed(3, 1, 0, 0, 1);

        //_imguiRenderer->RecordDrawCommands(renderingCommandBuffer);

        //renderingCommandBuffer.endRenderPass();

    //}

}