#include "FrameContext.hpp"

namespace eureka
{


    SwapChainFrameContext::SwapChainFrameContext(DeviceContext& deviceContext, Queue copyQueue, Queue graphicsQueue, std::shared_ptr<SwapChain> swapChain) :
        _deviceContext(deviceContext),
        _swapChain(std::move(swapChain)),
        _copyQueue(copyQueue),
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

}

