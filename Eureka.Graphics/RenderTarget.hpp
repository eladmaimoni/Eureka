#pragma once
#include "RenderPass.hpp"
#include "Image.hpp"
#include "SwapChain.hpp"
#include <debugger_trace.hpp>
#include "Commands.hpp"

namespace eureka
{
    inline constexpr vk::Format DEFAULT_DEPTH_BUFFER_FORMAT = vk::Format::eD24UnormS8Uint;

    class RenderTarget
    {
    public:
        virtual ~RenderTarget() { }
        RenderTarget(const vk::Rect2D& area, std::shared_ptr<RenderPass> renderPass, vkr::Framebuffer frameBuffer)
            : 
            _area(area),
            _renderPass(std::move(renderPass)),
            _frameBuffer(std::move(frameBuffer))
        {

        }
    
        RenderTarget(RenderTarget&& that) = default;
        RenderTarget& operator=(RenderTarget&& rhs) = default;

    protected:
        vk::Rect2D                   _area;
        std::shared_ptr<RenderPass>  _renderPass{ nullptr };
        vkr::Framebuffer             _frameBuffer{ nullptr };
    };

    class ColorRenderTarget : public RenderTarget
    {
    public:
        ColorRenderTarget(
            const vk::Rect2D& area,
            std::shared_ptr<RenderPass> renderPass,
            vkr::Framebuffer frameBuffer,
            std::shared_ptr<Image> outputColorImage
        )
            : 
            RenderTarget(area, std::move(renderPass), std::move(frameBuffer)),
            _outputColorImage(std::move(outputColorImage))
        {

        }

    protected:

        std::shared_ptr<Image> _outputColorImage;
    };


    class DepthColorRenderTarget : public RenderTarget
    {
    public:
        DepthColorRenderTarget(
            const vk::Rect2D& area,
            std::shared_ptr<RenderPass> renderPass,
            vkr::Framebuffer frameBuffer,
            std::shared_ptr<Image> outputColorImage,
            std::shared_ptr<Image2D> depthImage
        );
        const vk::RenderPassBeginInfo& BeginInfo() const { return _beginInfo; }

        EUREKA_DEFAULT_MOVEONLY(DepthColorRenderTarget);

    private:
        std::shared_ptr<Image>         _outputColorImage;
        std::shared_ptr<Image2D>       _depthImage;
        std::array< vk::ClearValue, 2> _clearValues{};
        vk::RenderPassBeginInfo        _beginInfo;
    };


    inline std::vector<DepthColorRenderTarget> CreateDepthColorTargetForSwapChain(
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
            std::array<vk::ImageView, 2> attachments = { images[i]->GetView(), depthImage->GetView() };

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

    struct FrameRecording
    {
        vk::CommandBuffer   command_buffer;
        vk::Semaphore       frame_available_wait_semaphore;
        //vk::Semaphore       frame_done_signal_semaphore;
        //vk::Fence           frame_done_signal_fence;
    };

    class SwapChainColorDepthFrameContext
    {
        /*
        this class represent a frame preparation
        - 
        
        */
    private:
        DeviceContext& _deviceContext;
        Queue _graphicsQueue;
        std::shared_ptr<SwapChain>                  _swapChain;
        std::vector<DepthColorRenderTarget>         _renderTargets;
        std::vector<FrameCommands>                  _frameCommands;
        uint32_t                                    _maxFramesInFlight{};
        sigslot::signal<uint32_t, uint32_t>         _resizeSignal;
        sigslot::scoped_connection                  _resizeConnection;
        std::shared_ptr<DepthColorRenderPass>       _renderPass;

        // current frame
        uint32_t                                    _currentFrame;
        vk::CommandBuffer                           _currentFrameCommandBuffer;
        vk::Semaphore                               _currentFrameDoneSignalSemaphore;
        vk::Fence                                   _currentFrameDoneSignalFence;
    public:
        std::shared_ptr<DepthColorRenderPass> GetRenderPass() const
        {
            return _renderPass;
        }

        SwapChainColorDepthFrameContext(DeviceContext& deviceContext, Queue graphicsQueue, std::shared_ptr<SwapChain> swapChain)
            : 
            _deviceContext(deviceContext),
            _swapChain(std::move(swapChain)),
            _graphicsQueue(graphicsQueue)
        {
            _maxFramesInFlight = _swapChain->ImageCount();

            
            // init commands
            for (auto i = 0u; i < _maxFramesInFlight; ++i)
            {
                _frameCommands.emplace_back(_deviceContext, _graphicsQueue);
            }

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
            _renderTargets = CreateDepthColorTargetForSwapChain(_deviceContext, *_swapChain, _renderPass);

            _resizeConnection = _swapChain->ConnectResizeSlot(
                [this](uint32_t w, uint32_t h)
                {
                    HandleSwapChainResize(w, h);
                }
            );
        }
        vk::Rect2D RenderArea() const
        {
            return _swapChain->RenderArea();
        }

        FrameRecording BeginFrameRecording()
        {
            auto [currentFrame, imageReadySemaphore] = _swapChain->AcquireNextAvailableImageAsync();
            // wait for current frame to finish execution before we reset its command pool (other frames can be in flight)
            auto& currentFrameCommandRecord = _frameCommands[currentFrame];
            _currentFrame = currentFrame;
            _currentFrameDoneSignalFence = currentFrameCommandRecord.DoneFence();
            _currentFrameDoneSignalSemaphore = currentFrameCommandRecord.DoneSemaphore();
            _currentFrameCommandBuffer = *currentFrameCommandRecord.CommandBuffer();

            WaitForFrame(_currentFrameDoneSignalFence);
            currentFrameCommandRecord.Reset(); // reset pool



            _currentFrameCommandBuffer.begin(vk::CommandBufferBeginInfo());

            return FrameRecording{ 
                _currentFrameCommandBuffer,
                imageReadySemaphore
            };
        }

        void BeginPrimaryRenderPass()
        {
            _currentFrameCommandBuffer.beginRenderPass(_renderTargets[_currentFrame].BeginInfo(), vk::SubpassContents::eInline);

        }

        void EndPrimaryRenderPass()
        {
            _currentFrameCommandBuffer.endRenderPass();
        }

        void EndFrameRecordingAndSubmit(
            dynamic_span<vk::Semaphore> waitList,
            dynamic_span<vk::PipelineStageFlags> waitStageMasks   
        )
        {
            _currentFrameCommandBuffer.end();

            vk::SubmitInfo submitInfo
            {
                .waitSemaphoreCount = static_cast<uint32_t>(waitList.size()),
                .pWaitSemaphores = waitList.data(),
                .pWaitDstStageMask = waitStageMasks.data(),
                .commandBufferCount = 1,
                .pCommandBuffers = &_currentFrameCommandBuffer,
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = &_currentFrameDoneSignalSemaphore
            };

            _graphicsQueue->submit(
                { submitInfo },
                _currentFrameDoneSignalFence
            );
        }



        void Present()
        {
            PROFILE_CATEGORIZED_SCOPE("Present", Profiling::Color::Gray, Profiling::PROFILING_CATEGORY_RENDERING);
            auto result = _swapChain->PresentLastAcquiredImageAsync(_currentFrameDoneSignalSemaphore);

            if (result != vk::Result::eSuccess)
            {
                DEBUGGER_TRACE("result = {}", result);
            }
        }

        template <typename Callable>
        sigslot::connection ConnectResizeSlot(Callable&& slot)
        {
            return _resizeSignal.connect(std::forward<Callable>(slot));
        }
    private:
        void WaitForFrame(vk::Fence currentFrameFence)
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


        void HandleSwapChainResize(uint32_t width, uint32_t height)
        {
            DEBUGGER_TRACE("handle swap chain resize");

            _graphicsQueue->waitIdle();
            _renderTargets = CreateDepthColorTargetForSwapChain(
                _deviceContext,
                *_swapChain,
                _renderPass
            );

            _resizeSignal(width, height);
        }
    };
}