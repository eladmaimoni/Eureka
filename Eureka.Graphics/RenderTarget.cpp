#include "RenderTarget.hpp"


namespace eureka
{

    constexpr std::array<float, 4> DEFAULT_CLEAR_COLOR{ 0.79f, 0.65f, 0.93f, 1.0f };
    
    DepthColorRenderTarget::DepthColorRenderTarget(
        const vk::Rect2D& area,
        std::shared_ptr<RenderPass> renderPass, 
        vkr::Framebuffer frameBuffer, 
        std::shared_ptr<Image> outputColorImage, 
        std::shared_ptr<Image2D> depthImage
    ) :
        RenderTarget(area, std::move(renderPass), std::move(frameBuffer)),
        _outputColorImage(std::move(outputColorImage)),
        _depthImage(std::move(depthImage))
    {
        _clearValues[0].color = vk::ClearColorValue{ DEFAULT_CLEAR_COLOR };
        _clearValues[1].depthStencil = vk::ClearDepthStencilValue{ 1.0f, 0 };

        _beginInfo = vk::RenderPassBeginInfo 
        {
            .renderPass = _renderPass->Get(),
            .framebuffer = *_frameBuffer,
            .renderArea = _area,
            .clearValueCount = 2,
            .pClearValues = _clearValues.data()
        };
    }



}