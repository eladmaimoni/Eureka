#include "TargetPass.hpp"

namespace eureka
{


    SwapChainDepthColorPass::SwapChainDepthColorPass(
        DeviceContext& deviceContext, 
        Queue graphicsQueue, 
        std::shared_ptr<SwapChain> swapChain
    ) :
        _deviceContext(deviceContext),
        _graphicsQueue(graphicsQueue),
        _swapChain(std::move(swapChain))
    {
        _maxFramesInFlight = _swapChain->ImageCount();


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


        _currentRenderTarget = &_renderTargets[0];

        _resizeConnection = _swapChain->ConnectResizeSlot(
            [this](uint32_t w, uint32_t h)
            {
                HandleSwapChainResize(w, h);
            }
        );

    }

    void SwapChainDepthColorPass::AddViewPass(std::shared_ptr<IViewPass> viewPass)
    {
        auto& vp = _viewPasses.emplace_back(std::move(viewPass));

        vp->HandleResize(_width, _height);
    }

    void SwapChainDepthColorPass::Prepare()
    {
        for (auto& view : _viewPasses)
        {
            view->Prepare();
        }
    }

    TargetPassBeginInfo SwapChainDepthColorPass::PreRecord()
    {
        auto [valid, currentFrame, imageReadySemaphore] = _swapChain->AcquireNextAvailableImageAsync();

        if (valid)
        {
            // wait for current frame to finish execution before we reset its command pool (other frames can be in flight)
            _currentRenderTarget = &_renderTargets[currentFrame];

            return TargetPassBeginInfo
            {
                .valid = true,
                .target_available_wait_semaphore = imageReadySemaphore
            };
        }
        else
        {
            return TargetPassBeginInfo
            {
                .valid = false
            };
        }
    }

    void SwapChainDepthColorPass::PostRecord()
    {

    }

    void SwapChainDepthColorPass::PostSubmit(vk::Semaphore waitSemaphore)
    {
        PROFILE_CATEGORIZED_SCOPE("Present", Profiling::Color::Gray, Profiling::PROFILING_CATEGORY_RENDERING);
        auto result = _swapChain->PresentLastAcquiredImageAsync(waitSemaphore);

        if (result == vk::Result::eErrorOutOfDateKHR)
        {
            _graphicsQueue->waitIdle();
            _renderTargets = CreateDepthColorTargetForSwapChain(
                _deviceContext,
                *_swapChain,
                _renderPass
            );


        }
    }

    void SwapChainDepthColorPass::HandleSwapChainResize(uint32_t width, uint32_t height)
    {
        _width = width;
        _height = height;
        DEBUGGER_TRACE("handle swap chain resize");

        _graphicsQueue->waitIdle();
        _renderTargets = CreateDepthColorTargetForSwapChain(
            _deviceContext,
            *_swapChain,
            _renderPass
        );


        for (auto& view : _viewPasses)
        {
            view->HandleResize(width, height);
        }


        _resizeSignal(width, height);
    }

    void SwapChainDepthColorPass::RecordDraw(const RecordParameters& params)
    {
        params.command_buffer.beginRenderPass(_currentRenderTarget->BeginInfo(), vk::SubpassContents::eInline);

        for (auto& view : _viewPasses)
        {
            view->RecordDraw(params);
        }

        params.command_buffer.endRenderPass();
    }

}

