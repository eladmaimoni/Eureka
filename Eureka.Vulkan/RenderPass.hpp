#pragma once
#include "Device.hpp"

namespace eureka::vulkan
{

    class RenderPass
    {
    protected:
        std::shared_ptr<Device> _device;
        VkRenderPass _renderPass{ nullptr };
    public:
        RenderPass(std::shared_ptr<Device> device)
            : _device(std::move(device))
        {

        }
        virtual ~RenderPass()
        {
            if (_renderPass)
            {
                _device->DestroyRenderPass(_renderPass);
            }
        }

        VkRenderPass Get() const { return _renderPass; }
    };

    struct DepthColorRenderPassConfig
    {
        VkFormat color_output_format;
        VkFormat depth_output_format;
    };

    class DepthColorRenderPass : public RenderPass
    {
        VkFormat _depthFormat;
    public:
        DepthColorRenderPass(std::shared_ptr<Device> device, const DepthColorRenderPassConfig& config);
        VkFormat DepthFormat() const { return _depthFormat; }
    };
}
