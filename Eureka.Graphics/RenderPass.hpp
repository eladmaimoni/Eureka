#include "DeviceContext.hpp"

namespace eureka
{
    struct ForwardRenderPassConfig
    {
        vk::Format color_output_format;
        vk::Format depth_output_format;
    };

    class ForwardRenderPass
    {
        ForwardRenderPass(const DeviceContext& deviceContext, const ForwardRenderPassConfig& config);


        vkr::RenderPass _renderPass{ nullptr };
    };
}
