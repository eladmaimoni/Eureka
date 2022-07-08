#include "RenderingSystem.hpp"
#include "SwapChain.hpp"
#include "RenderTarget.hpp"
#include "GraphicsDefaults.hpp"
#include <profiling_macros.hpp>
namespace eureka
{

    std::vector<DepthColorRenderTarget> CreateDepthColorTargetForSwapChain(
        const DeviceContext& deviceContext,
        const SwapChain& swapChain,
        const std::shared_ptr<DepthColorRenderPass>& renderPass
    )
    {
        std::vector<DepthColorRenderTarget> targets;
      
        // create depth image

        auto renderArea = swapChain.RenderArea();
        auto depthImage = std::make_shared<Image2D>(CreateDepthImage(deviceContext, renderPass->DepthFormat(), renderArea.extent.width, renderArea.extent.height));

        // create frame buffer
        auto images = swapChain.Images();
        targets.reserve(images.size());

        for (auto i = 0u; i < images.size(); ++i)
        {
            std::array<vk::ImageView, 2> attachments = { images[i]->GetView(), depthImage->GetView()};

            vk::FramebufferCreateInfo framebufferCreateInfo
            {
                .flags = vk::FramebufferCreateFlags(),
                .renderPass = renderPass->Get(),
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = renderArea.extent.width,
                .height = renderArea.extent.height,
                .layers = 1
            };

            auto framebuffer = deviceContext.LogicalDevice()->createFramebuffer(framebufferCreateInfo);

            targets.emplace_back(
                renderArea,
                renderPass,
                std::move(framebuffer),
                std::move(images[i]),
                depthImage
            );
        }
   
        return targets;
    }
}


namespace eureka
{

    RenderingSystem::RenderingSystem(
        DeviceContext& deviceContext,
        std::shared_ptr<SwapChain> swapChain,
        std::shared_ptr<SubmissionThreadExecutionContext> submissionThreadExecutionContext,
        std::shared_ptr<OneShotCopySubmissionHandler>     oneShotCopySubmissionHandler,
        std::shared_ptr<MTDescriptorAllocator>                   descPool,
        Queue graphicsQueue,
        Queue copyQueue
    )
        :
        _deviceContext(deviceContext),
        _swapChain(swapChain),
        _descPool(std::move(descPool)),
        _submissionThreadExecutionContext(/*std::move(*/submissionThreadExecutionContext/*)*/), // TODO
        _oneShotCopySubmissionHandler(std::move(oneShotCopySubmissionHandler)),
        _camera(deviceContext, /*std::move(*/submissionThreadExecutionContext/*)*/),  // TODO
        _graphicsQueue(graphicsQueue),
        _copyQueue(copyQueue)
    {
        _maxFramesInFlight = _swapChain->ImageCount();
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

        InitializeCommandPoolsAndBuffers();

        bool found = false;
        vk::Format depthFormat = DEFAULT_DEPTH_BUFFER_FORMAT;
        for (auto format : { vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint })
        {
            auto props = _deviceContext.PhysicalDevice()->getFormatProperties(format);

            if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
            {
                depthFormat = format;
                found = true;
                break;
            }
        }

        DepthColorRenderPassConfig depthColorConfig
        {
            .color_output_format = _swapChain->ImageFormat(),
            .depth_output_format = depthFormat
        };

        _renderPass = std::make_shared<DepthColorRenderPass>(_deviceContext, depthColorConfig);


        _stageZone = HostWriteCombinedBuffer(
            _deviceContext.Allocator(),
            BufferConfig{ .byte_size = sizeof(mesh::COLORED_TRIANGLE_INDEX_DATA) + sizeof(mesh::COLORED_TRIANGLE_VERTEX_DATA) }
        );



        _triangle = VertexAndIndexTransferableDeviceBuffer(
            _deviceContext.Allocator(),
            BufferConfig{ .byte_size = sizeof(mesh::COLORED_TRIANGLE_INDEX_DATA) + sizeof(mesh::COLORED_TRIANGLE_VERTEX_DATA) }
        );


        _lastFrameTime = std::chrono::high_resolution_clock::now();

        _pipelineCache = std::make_shared<PipelineCache>(_deviceContext, _renderPass);

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

        {
            HandleSwapChainResize();
        }

        _resizeConnection = _swapChain->ConnectResizeSlot(
            [this](uint32_t, uint32_t)
            {
                HandleSwapChainResize();
            }
        );
    }

    void RenderingSystem::HandleSwapChainResize()
    {
        DEBUGGER_TRACE("handle swap chain resize");
        //co_await concurrencpp::resume_on(_submissionThreadExecutionContext->PreRenderExecutor());

        _graphicsQueue->waitIdle();
        _renderTargets = CreateDepthColorTargetForSwapChain(
            _deviceContext, 
            *_swapChain,
            _renderPass
        );
        auto renderArea = _swapChain->RenderArea();
        _camera.SetFullViewport(renderArea.offset.x, renderArea.offset.y, renderArea.extent.width, renderArea.extent.height);
   
        RunOne(); // refresh before next resize

        //co_return;
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
    

        auto [currentFrame, imageReadySemaphore] = _swapChain->AcquireNextAvailableImageAsync();
        
        // wait for current frame to finish execution before we reset its command buffer (other frames can be in flight)
        auto& currentFrameCommandRecord = _frameCommandBuffer[currentFrame];
        auto currentFrameFence = currentFrameCommandRecord.DoneFence();
        WaitForFrame(currentFrameFence); 

        PROFILE_CATEGORIZED_SCOPE("Record Submit", Profiling::Color::DarkGray, Profiling::PROFILING_CATEGORY_RENDERING);
        currentFrameCommandRecord.Reset(); // reset pool
        auto& renderingCommandBuffer = currentFrameCommandRecord.CommandBuffer();
        // Record pre-frame here

        auto renderingDoneSemaphore = currentFrameCommandRecord.DoneSemaphore();
        RecordMainRenderPass(currentFrame,renderingCommandBuffer);       
        SubmitFrame(renderingCommandBuffer, imageReadySemaphore, renderingDoneSemaphore, currentFrameFence);

        PROFILE_CATEGORIZED_SCOPE("Present", Profiling::Color::Gray, Profiling::PROFILING_CATEGORY_RENDERING);
        auto result = _swapChain->PresentLastAcquiredImageAsync(renderingDoneSemaphore);

        if (result != vk::Result::eSuccess)
        {
            DEBUGGER_TRACE("result = {}", result);
        }
    }


    void RenderingSystem::WaitForFrame(vk::Fence currentFrameFence)
    {
        PROFILE_CATEGORIZED_SCOPE("WaitForFrame", Profiling::Color::DarkSeaGreen, Profiling::PROFILING_CATEGORY_RENDERING);
        VK_CHECK(_deviceContext.LogicalDevice()->waitForFences(
            { currentFrameFence },
            VK_TRUE,
            UINT64_MAX
        ));

        _deviceContext.LogicalDevice()->resetFences(
            { currentFrameFence }
        );
    }

    void RenderingSystem::SubmitFrame(const vkr::CommandBuffer& renderingCommandBuffer, vk::Semaphore imageReadySemaphore, vk::Semaphore renderingDoneSemaphore, vk::Fence renderingDoneFence)
    {
        std::array<vk::Semaphore, 1> waitSemaphores
        {
            imageReadySemaphore
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
            .pCommandBuffers = &*renderingCommandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &renderingDoneSemaphore
        };

        _graphicsQueue->submit(
            { submitInfo },
            renderingDoneFence
        );
    }

    void RenderingSystem::RecordMainRenderPass(uint32_t currentFrame, vkr::CommandBuffer& renderingCommandBuffer)
    {
        ScopedCommands sc(renderingCommandBuffer);

        renderingCommandBuffer.beginRenderPass(_renderTargets[currentFrame].BeginInfo(), vk::SubpassContents::eInline);


        renderingCommandBuffer.setViewport(0, { _camera.Viewport() });
        renderingCommandBuffer.setScissor(0, { _swapChain->RenderArea() });

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

        renderingCommandBuffer.endRenderPass();
    }



    void RenderingSystem::InitializeCommandPoolsAndBuffers()
    {
        for (auto i = 0u; i < _maxFramesInFlight; ++i)
        {          
            _frameCommandBuffer.emplace_back(_deviceContext, _graphicsQueue);
        }
    }



}