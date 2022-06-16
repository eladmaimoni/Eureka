#include "RenderPass.hpp"

namespace eureka
{
    //////////////////////////////////////////////////////////////////////////
    //
    //                      DepthColorRenderPass
    //
    //////////////////////////////////////////////////////////////////////////


    DepthColorRenderPass::DepthColorRenderPass(
        const DeviceContext& deviceContext,
        const DepthColorRenderPassConfig& config
    ) : _depthFormat(config.depth_output_format)
    {
        std::array<vk::AttachmentDescription, 2> colorAndDepthAttachments;

        //
        // AttachmentDescription
        //

        colorAndDepthAttachments[0] = vk::AttachmentDescription
        {
            .format = config.color_output_format,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::ePresentSrcKHR
        };


        colorAndDepthAttachments[1] = vk::AttachmentDescription
        {
            .format = config.depth_output_format,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal
        };

        //
        // AttachmentReference
        //

        vk::AttachmentReference colorAttachmentRefrence
        {
            .attachment = 0,
            .layout = vk::ImageLayout::eColorAttachmentOptimal
        };

        vk::AttachmentReference depthAttachmentRefrence
        {
            .attachment = 1,
            .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal
        };

        //
        // SubpassDescription & SubpassDependency
        //
        
        vk::SubpassDescription subpassDesc
        {
            .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRefrence,
            .pDepthStencilAttachment = &depthAttachmentRefrence
        };

        vk::SubpassDependency subpassDependency
        {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
            .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
            .srcAccessMask = {},
            .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
            .dependencyFlags = {},
        };


        //
        // SubpassDescription & SubpassDependency
        //

        vk::RenderPassCreateInfo renderPassCreateInfo
        {
            .attachmentCount = static_cast<uint32_t>(colorAndDepthAttachments.size()),
            .pAttachments = colorAndDepthAttachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpassDesc,
            .dependencyCount = 1,
            .pDependencies = &subpassDependency
        };

        _renderPass = deviceContext.LogicalDevice()->createRenderPass(renderPassCreateInfo);
    }

}
