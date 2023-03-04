#include "RenderingSystem.hpp"
//#include "SwapChain.hpp"
//#include "RenderTarget.hpp"
#include "GraphicsDefaults.hpp"
#include <profiling_macros.hpp>
#include "RenderDocIntegration.hpp"
#include <debugger_trace.hpp>

namespace eureka::graphics
{


    RenderingSystem::RenderingSystem(
        std::shared_ptr<vulkan::Device> device,
        vulkan::Queue graphicsQueue,
        vulkan::Queue copyQueue,
        std::shared_ptr<vulkan::FrameContext> frameContext,
        std::shared_ptr<ITargetPass> mainPass,
        std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext,
        std::shared_ptr<OneShotSubmissionHandler> oneShotSubmissionHandler
    )
        :
        _device(std::move(device)),
        _graphicsQueue(graphicsQueue),
        _copyQueue(copyQueue),
        _frameContext(std::move(frameContext)),
        _submissionThreadExecutionContext(/*std::move(*/submissionThreadExecutionContext/*)*/), // TODO
        _oneShotSubmissionHandler(std::move(oneShotSubmissionHandler)),
        _mainPass(std::move(mainPass))
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

        _resizeConnection = _mainPass->ConnectResizeSlot(
            [this](uint32_t w, uint32_t h)
        {
            HandleResize(w, h);
        });
        auto [width, height] = _mainPass->GetSize();
        HandleResize(width, height);

    }

    void RenderingSystem::HandleResize(uint32_t /*w*/, uint32_t /*h*/)
    {
        //DEBUGGER_TRACE("handle swap chain resize");

        RunOne(); // refresh before next resize
    }

    void RenderingSystem::Deinitialize()
    {
        _graphicsQueue.WaitIdle();
        _copyQueue.WaitIdle();
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
            PROFILE_CATEGORIZED_SCOPE("RunOne", eureka::profiling::Color::Blue, eureka::profiling::PROFILING_CATEGORY_RENDERING);

            //_graphicsQueue->waitIdle();
            _frameContext->BeginFrame();

            _mainPass->Prepare();

            _submissionThreadExecutionContext->PreRenderExecutor().loop(100);

            auto [valid, targetReady] = _mainPass->PreRecord();
            if (!valid)
            {
                return;
            }

            auto [mainCommandBuffer, doneSemaphore] = _frameContext->NewGraphicsPresentCommandBuffer();
            mainCommandBuffer.Begin();

            _mainPass->RecordDraw({ mainCommandBuffer });
            _mainPass->PostRecord();

            mainCommandBuffer.End();

            std::array<VkSemaphore, 1> waitSemaphores
            {
                targetReady
            };
         
            std::array<VkPipelineStageFlags, 1> waitStageMasks
            {
                VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
            };

            auto doneSemaphoreHandle = doneSemaphore.Get();
            
            auto mainCommandBufferHandle = mainCommandBuffer.Get();
            VkSubmitInfo submitInfo
            {
                .sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
                .pWaitSemaphores = waitSemaphores.data(),
                .pWaitDstStageMask = waitStageMasks.data(),
                .commandBufferCount = 1,
                .pCommandBuffers = &mainCommandBufferHandle,
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = &doneSemaphoreHandle
            };
     
            _graphicsQueue.Submit(submitInfo, _frameContext->NewGraphicsSubmitFence());
           
            // On Intel (single queue), we have a race with the presentation engine
            // https://stackoverflow.com/questions/63320119/vksubpassdependency-specification-clarification
            // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkQueuePresentKHR.html
            // https://stackoverflow.com/questions/68050676/can-vkqueuepresentkhr-be-synced-using-a-pipeline-barrier
            _mainPass->PostSubmit(doneSemaphore);
            //_graphicsQueue->waitIdle();
            _frameContext->EndFrame();    


        }
        catch (const std::exception& err)
        {
            DEBUGGER_TRACE("RUN ONE error {}", err.what());
        }
        //DEBUGGER_TRACE("RUN ONE END");
    }



    void RenderingSystem::PollTasks()
    {
        _submissionThreadExecutionContext->Executor().loop_all(MAX_COPY_SUBMITS_PER_FRAME);
        _oneShotSubmissionHandler->SubmitPendingCopies();
        _oneShotSubmissionHandler->PollCopyCompletions();
        _oneShotSubmissionHandler->SubmitPendingGraphics();
        _oneShotSubmissionHandler->PollGraphicsCompletions();
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