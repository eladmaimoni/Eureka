#include "RenderingSystem.hpp"
#include "SwapChain.hpp"
#include "RenderTarget.hpp"
#include "GraphicsDefaults.hpp"

namespace eureka
{

    std::vector<DepthColorRenderTarget> CreateDepthColorTargetForSwapChain(
        const DeviceContext& deviceContext,
        const SwapChain& swapChain,
        const std::shared_ptr<RenderPass>& renderPass
    )
    {
        std::vector<DepthColorRenderTarget> targets;
      
        // create depth image

        auto renderArea = swapChain.RenderArea();
        auto depthImage = std::make_shared<Image2D>(CreateDepthImage(deviceContext, renderArea.extent.width, renderArea.extent.height));

        // create frame buffer
        auto images = swapChain.Images();
        targets.reserve(images.size());

        for (auto i = 0u; i < images.size(); ++i)
        {
            std::array<vk::ImageView, 2> attachments = { images[i]->View(), depthImage->View()};

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
    inline constexpr int DEFAULT_WINDOW_WIDTH = 1024;
    inline constexpr int DEFAULT_WINDOW_HEIGHT = 768;



    RenderingSystem::RenderingSystem(
        Instance& instance,
        DeviceContext& deviceContext,
        GLFWRuntime& glfw
    )
        :
        _glfw(glfw),
        _instance(instance),
        _deviceContext(deviceContext),
        _uploadPool(deviceContext.LogicalDevice())
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
        //
        // this section should be moved to some sort of window class
        //

        auto windowSurface = _glfw.CreateVulkanWindowSurface(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, _instance.Get());
        _deviceContext.InitializePresentationQueueFromExistingQueues(*windowSurface.surface);

       

        _window = std::move(windowSurface.window);

        glfwSetWindowUserPointer(_window.get(), this);
        glfwSetWindowSizeCallback(_window.get(), [](GLFWwindow* window, int width, int height)
            {
                auto userPtr = glfwGetWindowUserPointer(window);
                auto self = static_cast<RenderingSystem*>(userPtr);
                self->HandleResize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
            }
        );



        _presentationQueue = _deviceContext.PresentQueue();
        _graphicsQueue = _deviceContext.GraphicsQueue().front();
        _uploadQueue =_deviceContext.CopyQueue().at(0);

        InitializeSwapChain(windowSurface);

        InitializeCommandPoolsAndBuffers();

        DepthColorRenderPassConfig depthColorConfig
        {
            .color_output_format = _swapChain->ImageFormat(),
            .depth_output_format = DEFAULT_DEPTH_BUFFER_FORMAT
        };

        _renderPass = std::make_shared<DepthColorRenderPass>(_deviceContext, depthColorConfig);

        _renderTargets = CreateDepthColorTargetForSwapChain(
            _deviceContext,
            *_swapChain,
            _renderPass
        );
     
        _stageZone = HostStageZoneBuffer(
            _deviceContext, 
            BufferConfig { .byte_size = sizeof(mesh::COLORED_TRIANGLE_INDEX_DATA) + sizeof(mesh::COLORED_TRIANGLE_VERTEX_DATA)}        
        );

        _triangle = VertexAndIndexTransferableDeviceBuffer(
            _deviceContext,
            BufferConfig{ .byte_size = sizeof(mesh::COLORED_TRIANGLE_INDEX_DATA) + sizeof(mesh::COLORED_TRIANGLE_VERTEX_DATA) }
        );

        _uploadPool = CommandPool(_deviceContext.LogicalDevice(), CommandPoolDesc{.queue_family = _deviceContext.Families().copy_family_index});
        _uploadDoneSemaphore = _deviceContext.LogicalDevice()->createSemaphore(vk::SemaphoreCreateInfo());
        _uploadCommandBuffer = _uploadPool.AllocatePrimaryCommandBuffer();
        _uploadDoneFence = _deviceContext.LogicalDevice()->createFence(vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
        _lastFrameTime = std::chrono::high_resolution_clock::now();

        _perFrameDescriptorSet = std::make_shared<PerFrameGeneralPurposeDescriptorSet>(_deviceContext);
        _coloredVertexPipeline = ColoredVertexMeshPipeline(_deviceContext, _renderPass, _perFrameDescriptorSet);

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

    void RenderingSystem::RunOne()
    {
        auto [currentFrame, imageReadySemaphore] = _swapChain->AcquireNextAvailableImageAsync();
        

        //
        // wait for current frame to finish execution before we reset its command buffer (other frames can be in flight)
        //

        auto& currentFrameCommandRecord = _frameCommandBuffer[currentFrame];

        auto currentFrameFence = currentFrameCommandRecord.DoneFence();

        VK_CHECK(_deviceContext.LogicalDevice()->waitForFences(
            { currentFrameFence, *_uploadDoneFence},
            VK_TRUE,
            UINT64_MAX
        ));


        _deviceContext.LogicalDevice()->resetFences(
            { currentFrameFence, *_uploadDoneFence }
        );
        currentFrameCommandRecord.Reset();
        _uploadPool.Reset();
        {
            // this section could happen in a different thread (upload thread)

            _stageZone.Assign(std::span(mesh::COLORED_TRIANGLE_INDEX_DATA), 0);
            _stageZone.Assign(std::span(mesh::COLORED_TRIANGLE_VERTEX_DATA), sizeof(mesh::COLORED_TRIANGLE_INDEX_DATA));

            {
                ScopedCommands commands(_uploadCommandBuffer);

                _uploadCommandBuffer.copyBuffer(
                    _stageZone.Buffer(),
                    _triangle.Buffer(),
                    { vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = _triangle.ByteSize()} }
                );
            }

            vk::SubmitInfo uploadsSubmitInfo
            {
                .waitSemaphoreCount = 0,
                .pWaitSemaphores = nullptr,
                .pWaitDstStageMask = {},
                .commandBufferCount = 1,
                .pCommandBuffers = &*_uploadCommandBuffer,
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = &*_uploadDoneSemaphore
            };


            _uploadQueue.submit(uploadsSubmitInfo, *_uploadDoneFence);


        }


       
        auto now = std::chrono::high_resolution_clock::now();

        //DEBUGGER_TRACE("current frame = {}, interval = {}ms", currentFrame, std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastFrameTime).count());

        _lastFrameTime = now;



        //
        // record frame
        //

        auto& commandBuffer = currentFrameCommandRecord.CommandBuffer();

        commandBuffer.begin(vk::CommandBufferBeginInfo());

        commandBuffer.beginRenderPass(_renderTargets[currentFrame].BeginInfo(), vk::SubpassContents::eInline);

        commandBuffer.endRenderPass();

        commandBuffer.end();

        //
        // submit rendering
        //

        auto renderingDoneSemaphore = currentFrameCommandRecord.DoneSemaphore();


        std::array<vk::Semaphore, 2> waitSemaphores
        {
            *_uploadDoneSemaphore,imageReadySemaphore
        };


        std::array<vk::PipelineStageFlags, 2> waitStageMasks
        {
            vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eVertexInput
        };

        vk::SubmitInfo submitInfo
        {
            .waitSemaphoreCount = 2,
            .pWaitSemaphores = waitSemaphores.data(),
            .pWaitDstStageMask = waitStageMasks.data(),
            .commandBufferCount = 1,
            .pCommandBuffers = &*commandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &renderingDoneSemaphore
        };

        _graphicsQueue.submit(
            { submitInfo },
            currentFrameFence
        );

        auto result = _swapChain->PresentLastAcquiredImageAsync(renderingDoneSemaphore);

        if (result != vk::Result::eSuccess)
        {
            DEBUGGER_TRACE("result = {}", result);
        }
    }


    void RenderingSystem::InitializeSwapChain(GLFWVulkanSurface& windowSurface)
    {
        SwapChainTargetConfig swapChainDesc{};
        swapChainDesc.width = windowSurface.size.width;
        swapChainDesc.height = windowSurface.size.height;
        swapChainDesc.surface = std::move(windowSurface.surface);

        swapChainDesc.present_queue_family = _deviceContext.Families().present_family_index;
        swapChainDesc.graphics_queue_family = _deviceContext.Families().direct_graphics_family_index;

        _swapChain = std::make_unique<SwapChain>(_deviceContext, std::move(swapChainDesc));

        _maxFramesInFlight = _swapChain->ImageCount();
    }

    void RenderingSystem::InitializeCommandPoolsAndBuffers()
    {
        for (auto i = 0u; i < _maxFramesInFlight; ++i)
        {          
            _frameCommandBuffer.emplace_back(_deviceContext);
        }
    }

    void RenderingSystem::HandleResize(uint32_t width, uint32_t height)
    {
        DEBUGGER_TRACE("HandleResize({},{})", width, height);
        
        // recreate swap chain images and views
        _swapChain->Resize(width, height);

        // recreate all resizeable stuff
        _renderTargets = CreateDepthColorTargetForSwapChain(
            _deviceContext,
            *_swapChain,
            _renderPass
            );

        RunOne();
    }

}