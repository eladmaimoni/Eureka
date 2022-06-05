#include "RenderingSystem.hpp"
#include "SwapChain.hpp"
#include "RenderTarget.hpp"
#include <debugger_trace.hpp>
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
        _deviceContext(deviceContext)
    {

    }

    RenderingSystem::~RenderingSystem()
    {

    }

    void RenderingSystem::RunOne()
    {
        auto [currentFrame, imageReadySemaphore] = _swapChain->AcquireNextAvailableImageAsync();


        //
        // wait for current frame to finish execution before we reset its command buffer (other frames can be in flight)
        //

        auto currentFrameFence = *_renderingDoneFence[currentFrame];

        VK_CHECK(_deviceContext.LogicalDevice()->waitForFences(
            { currentFrameFence },
            VK_TRUE,
            UINT64_MAX
        ));

        _deviceContext.LogicalDevice()->resetFences(
            { currentFrameFence }
        );

        _mainGraphicsCommandPools[currentFrame].reset();

        //
        // record frame
        //

        auto& commandBuffer = _mainGraphicsCommandBuffers[currentFrame];

        commandBuffer.begin(vk::CommandBufferBeginInfo());

        commandBuffer.beginRenderPass(_renderTargets[currentFrame].BeginInfo(), vk::SubpassContents::eInline);

        commandBuffer.endRenderPass();

        commandBuffer.end();

        //
        // submit rendering
        //
        vk::PipelineStageFlags waitStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

        auto renderingDoneSemaphore = *_renderingDoneSemaphore[currentFrame];

        vk::SubmitInfo submitInfo
        {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &imageReadySemaphore,
            .pWaitDstStageMask = &waitStageMask,
            .commandBufferCount = 1,
            .pCommandBuffers = &*commandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &renderingDoneSemaphore

        };

        _graphicsQueue->submit(
            { submitInfo },
            currentFrameFence
        );

        auto result = _swapChain->PresentLastAcquiredImageAsync(renderingDoneSemaphore);

        if (result != vk::Result::eSuccess)
        {
            DEBUGGER_TRACE("result = {}", result);
        }

        //_presentationQueue->presentKHR()
        // record renderpass binding
        //commandBuffer.beginRenderPass(
        //    vk::RenderPassBeginInfo
        //    {
        //        .renderPass = nullptr,
        //        .framebuffer = nullptr,
        //        .renderArea = _swapChain->RenderArea()
        //    },
        //    vk::SubpassContents::eInline
        //);

        //_currentFrame = (_currentFrame + 1) % _maxFramesInFlight;
    

    }

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

        InitializeSwapChain(windowSurface);

        InitializeCommandPoolsAndBuffers();


        vk::SemaphoreCreateInfo semaphoreCreateInfo{};
        vk::FenceCreateInfo fenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };

        for (auto i = 0u; i < _maxFramesInFlight; ++i)
        {
            _renderingDoneFence.emplace_back(_deviceContext.LogicalDevice()->createFence(fenceCreateInfo));
            _renderingDoneSemaphore.emplace_back(_deviceContext.LogicalDevice()->createSemaphore(semaphoreCreateInfo));
        }

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

    }

    void RenderingSystem::InitializeSwapChain(GLFWVulkanSurface& windowSurface)
    {
        SwapChainTargetConfig swapChainDesc{};
        swapChainDesc.width = windowSurface.size.width;
        swapChainDesc.height = windowSurface.size.height;
        swapChainDesc.surface = std::move(windowSurface.surface);

        swapChainDesc.present_queue_family = _deviceContext.Families().present_family_index;
        swapChainDesc.graphics_queue_family = _deviceContext.Families().direct_graphics_family_index;

        _swapChain = std::make_unique<SwapChain>(_deviceContext,_presentationQueue, std::move(swapChainDesc));

        _maxFramesInFlight = _swapChain->ImageCount();
    }

    void RenderingSystem::InitializeCommandPoolsAndBuffers()
    {
        vk::CommandPoolCreateInfo graphicsThreadCommandPoolCreateInfo
        {
            .flags = {}, // no flags, means we can only reset the pool and not the command buffers
            .queueFamilyIndex = _deviceContext.Families().direct_graphics_family_index
        };


        for (auto i = 0u; i < _maxFramesInFlight; ++i)
        {            
            auto& pool = _mainGraphicsCommandPools.emplace_back(*_deviceContext.LogicalDevice(), graphicsThreadCommandPoolCreateInfo);

            vk::CommandBufferAllocateInfo graphicsCommandBufferAllocateInfo
            {
                .commandPool = *pool,
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = 1
            };

            _mainGraphicsCommandBuffers.emplace_back(
                std::move(_deviceContext.LogicalDevice()->allocateCommandBuffers(graphicsCommandBufferAllocateInfo).at(0))
            );
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