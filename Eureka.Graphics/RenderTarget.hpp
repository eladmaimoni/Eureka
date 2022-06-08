#pragma once
#include "RenderPass.hpp"
#include "Image.hpp"

namespace eureka
{
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



    private:
        std::shared_ptr<Image>         _outputColorImage;
        std::shared_ptr<Image2D>       _depthImage;
        std::array< vk::ClearValue, 2> _clearValues{};
        vk::RenderPassBeginInfo        _beginInfo;
    };
}