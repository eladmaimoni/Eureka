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

        
        // the first stage in the render pass is attachment layout transition for depth and color images
        // followed by a clear
        // we can't read / write to those images before image transition is done
        // 
        // for the depth buffer, we need to wait until all previously commands are done using its content
        // since we use a single depth buffer
        // therefore, we need to wait until all previously issued command finish writing to the depth /

        vk::SubpassDependency subpassDependency
        {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = vk::PipelineStageFlagBits::eLateFragmentTests, // wait for previously issued depth tests to finish, note we don't wait for color
            .dstStageMask = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eColorAttachmentOutput, // don't start depth test / color clear before previously depth writes are done and subpass image transition is done
            .srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite, // previous render pass writes to the same depth buffer
            .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
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
