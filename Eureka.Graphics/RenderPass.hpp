#pragma once
#include "DeviceContext.hpp"

namespace eureka
{


    class RenderPass
    {
    public:
        virtual ~RenderPass()
        {

        }

        vk::RenderPass Get() const { return *_renderPass; }

    protected:
        vkr::RenderPass _renderPass{ nullptr };
    };

    struct DepthColorRenderPassConfig
    {
        vk::Format color_output_format;
        vk::Format depth_output_format;
    };

    class DepthColorRenderPass : public RenderPass
    {
    public:
        DepthColorRenderPass(const DeviceContext& deviceContext, const DepthColorRenderPassConfig& config);

    };
}
