#pragma once
#include "RenderPass.hpp"
#include "Image.hpp"
#include "SwapChain.hpp"
#include <debugger_trace.hpp>
#include "Commands.hpp"

namespace eureka::vulkan
{


    class FrameBuffer
    {
        std::shared_ptr<Device> _device;
        VkFramebuffer _frameBuffer{nullptr};
    public:
        FrameBuffer(std::shared_ptr<Device> device, const VkFramebufferCreateInfo& createInfo)
            : _device(std::move(device))
        {
            _frameBuffer = _device->CreateFrameBuffer(createInfo);
        }
        FrameBuffer(FrameBuffer&& that) noexcept
            : _device(std::move(that._device)),
            _frameBuffer(std::move(that._frameBuffer))
        {
            that._frameBuffer = nullptr;
        }

        ~FrameBuffer()
        {
            if (_frameBuffer)
            {
                _device->DestroyFrameBuffer(_frameBuffer);
            }
        }
        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer& operator=(const FrameBuffer&){
            throw std::logic_error("The method or operation is not implemented.");
        }

        VkFramebuffer Get() const
        {
            return _frameBuffer;
        }
    };

    class RenderTarget
    {
    public:
        virtual ~RenderTarget() { }
        RenderTarget(const VkRect2D& area, std::shared_ptr<RenderPass> renderPass, FrameBuffer frameBuffer)
            : 
            _area(area),
            _renderPass(std::move(renderPass)),
            _frameBuffer(std::move(frameBuffer))
        {

        }
    
        RenderTarget(RenderTarget&& that) = default;
        RenderTarget& operator=(RenderTarget&& rhs) = default;

        const VkRenderPassBeginInfo& BeginInfo() const { return _beginInfo; }
    protected:
        VkRect2D                     _area;
        std::shared_ptr<RenderPass>  _renderPass{ nullptr };
        FrameBuffer                  _frameBuffer;
        VkRenderPassBeginInfo        _beginInfo{ VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, };
    };


    class DepthColorRenderTarget : public RenderTarget
    {
        std::shared_ptr<Image>         _outputColorImage;
        std::shared_ptr<AllocatedImage2D>       _depthImage;
        std::array< VkClearValue, 2> _clearValues{};
    public:
        DepthColorRenderTarget(
            const VkRect2D& area,
            std::shared_ptr<RenderPass> renderPass,
            FrameBuffer frameBuffer,
            std::shared_ptr<Image> outputColorImage,
            std::shared_ptr<AllocatedImage2D> depthImage
        );
    };


    std::vector<DepthColorRenderTarget> CreateDepthColorTargetForSwapChain(
        const std::shared_ptr<Device>& device,
        std::shared_ptr<ResourceAllocator> allocator,
        const SwapChain& swapChain,
        const std::shared_ptr<DepthColorRenderPass>& renderPass
    );


}