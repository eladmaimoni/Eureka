#include "RenderTarget.hpp"


namespace eureka
{

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
        _clearValues[0].color = vk::ClearColorValue{ std::array<float, 4> {0.0f, 1.0f, 0.2f, 1.0f} };
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