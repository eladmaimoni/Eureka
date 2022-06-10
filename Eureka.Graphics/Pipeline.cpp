#include "Pipeline.hpp"

namespace eureka
{

    PerFrameGeneralPurposeDescriptorSet::PerFrameGeneralPurposeDescriptorSet(DeviceContext& deviceContext)
    {
        // describe the relation between the shader indices (set 0, binding 0)
        // to the host indices 
        // - host indices are the sets
        // - device indices are the set index and binding number within the set that can potentially be unordered
        // set 0, only has a single binding inside shader
        vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding
        {
            .binding = 0, // shader side index (why not named location??)
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = 1, // a single constant buffer
            .stageFlags = vk::ShaderStageFlagBits::eVertex
        };

        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo
        {
            .bindingCount = 1,
            .pBindings = &descriptorSetLayoutBinding
        };

        _descriptorSetLayout = deviceContext.LogicalDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
    }

    ColoredVertexMeshPipeline::ColoredVertexMeshPipeline(DeviceContext& deviceContext, std::shared_ptr<DepthColorRenderPass> renderPass, std::shared_ptr<PerFrameGeneralPurposeDescriptorSet> descriptorSetLayout) :
        _descriptorSetLayout(descriptorSetLayout),
        _renderPass(renderPass)
    {
        auto layoutHandle = _descriptorSetLayout->Get();
        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo
        {
            .setLayoutCount = 1,
            .pSetLayouts = &layoutHandle
        };

        _pipelineLayout = deviceContext.LogicalDevice()->createPipelineLayout(pipelineLayoutCreateInfo);

        Setup(deviceContext);
    }

    void ColoredVertexMeshPipeline::Setup(DeviceContext& deviceContext)
    {
        //
        // Setup States
        //

        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState
        {
            .topology = vk::PrimitiveTopology::eTriangleList
        };

        vk::PipelineRasterizationStateCreateInfo rasterizationState
        {
            .depthClampEnable = false,
            .rasterizerDiscardEnable = false,
            .polygonMode = vk::PolygonMode::eFill,
            .cullMode = vk::CullModeFlagBits::eNone,
            .frontFace = vk::FrontFace::eCounterClockwise,
            .depthBiasEnable = false,
            .lineWidth = 1.0f
        };

        vk::PipelineColorBlendAttachmentState blendAttachments
        {
            .blendEnable = false,
            .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        };

        vk::PipelineColorBlendStateCreateInfo blendState
        {
            .attachmentCount = 1,
            .pAttachments = &blendAttachments
        };

        vk::PipelineViewportStateCreateInfo viewportState // overriden by dynamic state
        {
            .viewportCount = 1,
            .scissorCount = 1
        };

        std::array<vk::DynamicState, 2> enabledDynamicStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };


        vk::PipelineDynamicStateCreateInfo dynamicState
        {
            .dynamicStateCount = static_cast<uint32_t>(enabledDynamicStates.size()),
            .pDynamicStates = enabledDynamicStates.data()
        };

        vk::PipelineDepthStencilStateCreateInfo depthStencilState
        {
            .depthTestEnable = true,
            .depthWriteEnable = true,
            .depthCompareOp = vk::CompareOp::eLessOrEqual,
            .depthBoundsTestEnable = false,
            .stencilTestEnable = false,
            .front = vk::StencilOpState{.failOp = vk::StencilOp::eKeep,.passOp = vk::StencilOp::eKeep, .depthFailOp = vk::StencilOp::eKeep, .compareOp = vk::CompareOp::eAlways },
            .back = vk::StencilOpState{.failOp = vk::StencilOp::eKeep,.passOp = vk::StencilOp::eKeep, .depthFailOp = vk::StencilOp::eKeep, .compareOp = vk::CompareOp::eAlways }
        };

        vk::PipelineMultisampleStateCreateInfo multisampleState
        {
            .rasterizationSamples = vk::SampleCountFlagBits::e1,
            .pSampleMask = nullptr
        };

        //
        // Vertex Attributes 
        //

        vk::VertexInputBindingDescription vertexInputBinding
        {
            .binding = 0,
            .stride = sizeof(PositionColorVertex),
            .inputRate = vk::VertexInputRate::eVertex
        };

        std::array<vk::VertexInputAttributeDescription, 2> vertexInputAttributs
        {
            // xyz position at shader location 0
            vk::VertexInputAttributeDescription
            {
                .location = 0,
                .binding = 0,
                .format = vk::Format::eR32G32B32Sfloat,
                .offset = offsetof(PositionColorVertex, position)
            },
            // rgb color at shader location 1
            vk::VertexInputAttributeDescription
            {
                .location = 1,
                .binding = 0,
                .format = vk::Format::eR32G32B32Sfloat,
                .offset = offsetof(PositionColorVertex, color)
            }
        };

        vk::PipelineVertexInputStateCreateInfo vertexInputState
        {
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &vertexInputBinding,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributs.size()),
            .pVertexAttributeDescriptions = vertexInputAttributs.data()
        };

        //
        // Shaders
        // 

        auto vshader = deviceContext.Shaders().LoadShaderModule(ColoredVertexVS);
        auto fshader = deviceContext.Shaders().LoadShaderModule(ColoredVertexFS);

        std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages
        {
            vk::PipelineShaderStageCreateInfo
            {
                .stage = vk::ShaderStageFlagBits::eVertex,
                .module = *vshader,
                .pName = "main"
            },
            vk::PipelineShaderStageCreateInfo
            {
                .stage = vk::ShaderStageFlagBits::eFragment,
                .module = *fshader,
                .pName = "main"
            }
        };


        //
        // All Together
        //

        vk::GraphicsPipelineCreateInfo pipelineCreateInfo
        {
            .stageCount = static_cast<uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputState,
            .pInputAssemblyState = &inputAssemblyState,
            .pTessellationState = nullptr,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizationState,
            .pMultisampleState = &multisampleState,
            .pDepthStencilState = &depthStencilState,
            .pColorBlendState = &blendState,
            .pDynamicState = &dynamicState,
            .layout = *_pipelineLayout,
            .renderPass = _renderPass->Get()
        };

        _pipeline = deviceContext.LogicalDevice()->createGraphicsPipeline(
            deviceContext.Shaders().Cache(),
            pipelineCreateInfo
        );
    }

}