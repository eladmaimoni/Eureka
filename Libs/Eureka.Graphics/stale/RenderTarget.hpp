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

        const vk::RenderPassBeginInfo& BeginInfo() const { return _beginInfo; }
    protected:
        vk::Rect2D                   _area;
        std::shared_ptr<RenderPass>  _renderPass{ nullptr };
        vkr::Framebuffer             _frameBuffer{ nullptr };
        vk::RenderPassBeginInfo      _beginInfo;
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


        EUREKA_DEFAULT_MOVEONLY(DepthColorRenderTarget);

    private:
        std::shared_ptr<Image>         _outputColorImage;
        std::shared_ptr<Image2D>       _depthImage;
        std::array< vk::ClearValue, 2> _clearValues{};
     
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


}