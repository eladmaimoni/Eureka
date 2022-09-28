#include "RenderPass.hpp"
#include <array>

namespace eureka::vulkan
{
    //////////////////////////////////////////////////////////////////////////
    //
    //                      DepthColorRenderPass
    //
    //////////////////////////////////////////////////////////////////////////


    DepthColorRenderPass::DepthColorRenderPass(
        std::shared_ptr<Device> device,
        const DepthColorRenderPassConfig& config
    ) : RenderPass(std::move(device)), _depthFormat(config.depth_output_format)
    {
        std::array<VkAttachmentDescription, 2> colorAndDepthAttachments;

        //
        // AttachmentDescription
        //

        colorAndDepthAttachments[0] = VkAttachmentDescription
        {
            .format = config.color_output_format,
            .samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };


        colorAndDepthAttachments[1] = VkAttachmentDescription
        {
            .format = config.depth_output_format,
            .samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };

        //
        // AttachmentReference
        //

        VkAttachmentReference colorAttachmentRefrence
        {
            .attachment = 0,
            .layout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        VkAttachmentReference depthAttachmentRefrence
        {
            .attachment = 1,
            .layout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };

        //
        // SubpassDescription & SubpassDependency
        //

        VkSubpassDescription subpassDesc
        {
            .pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS,
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

        VkSubpassDependency subpassDependency
        {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, // wait for previously issued depth tests to finish, note we don't wait for color
            .dstStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // don't start depth test / color clear before previously depth writes are done and subpass image transition is done
            .srcAccessMask = VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, // previous render pass writes to the same depth buffer
            .dstAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = {},
        };
        //
        // SubpassDescription & SubpassDependency
        //

        VkRenderPassCreateInfo renderPassCreateInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = static_cast<uint32_t>(colorAndDepthAttachments.size()),
            .pAttachments = colorAndDepthAttachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpassDesc,
            .dependencyCount = 1,
            .pDependencies = &subpassDependency
        };

        _renderPass = _device->CreateRenderPass(renderPassCreateInfo);
    }

}
