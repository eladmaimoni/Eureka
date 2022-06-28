#include "Pipeline.hpp"
#include <ShadersCache.hpp>

namespace eureka
{
    struct FixedPiplinePreset
    {
        vk::PipelineInputAssemblyStateCreateInfo input_assembly_create_info;
        vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info;
        vk::PipelineColorBlendStateCreateInfo    color_blend_state_create_info;
        vk::PipelineViewportStateCreateInfo      viewport_state_create_info;
        vk::PipelineDynamicStateCreateInfo       dynamic_state_create_info;
        vk::PipelineDepthStencilStateCreateInfo  depth_stencil_state_create_info;
        vk::PipelineMultisampleStateCreateInfo   multisampling_state_create_info;
    };

    struct MeshPipelinePreset
    {
        vk::PipelineColorBlendAttachmentState color_blend_attachment_state;
        std::array<vk::DynamicState, 2>       enabled_dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
        FixedPiplinePreset                    fixed_preset;

        void Setup()
        {

        }
    };


    void SetupFixedPreset(MeshPipelinePreset& meshPipelinePreset)
    {
        meshPipelinePreset.fixed_preset = FixedPiplinePreset
        {
            .input_assembly_create_info = vk::PipelineInputAssemblyStateCreateInfo
            {
                .topology = vk::PrimitiveTopology::eTriangleList
            },
            .rasterization_state_create_info = vk::PipelineRasterizationStateCreateInfo
            {
                .depthClampEnable = false,
                .rasterizerDiscardEnable = false,
                .polygonMode = vk::PolygonMode::eFill,
                .cullMode = vk::CullModeFlagBits::eNone,
                .frontFace = vk::FrontFace::eCounterClockwise,
                .depthBiasEnable = false,
                .lineWidth = 1.0f
            },
            .color_blend_state_create_info = vk::PipelineColorBlendStateCreateInfo
            {
                .attachmentCount = 1,
                .pAttachments = &meshPipelinePreset.color_blend_attachment_state
            },
            .viewport_state_create_info = vk::PipelineViewportStateCreateInfo
            {
                .viewportCount = 1,
                .scissorCount = 1
            },
            .dynamic_state_create_info = vk::PipelineDynamicStateCreateInfo
            {
                .dynamicStateCount = static_cast<uint32_t>(meshPipelinePreset.enabled_dynamic_states.size()),
                .pDynamicStates = meshPipelinePreset.enabled_dynamic_states.data()
            },
            .depth_stencil_state_create_info = vk::PipelineDepthStencilStateCreateInfo
            {
                .depthTestEnable = true,
                .depthWriteEnable = true,
                .depthCompareOp = vk::CompareOp::eLessOrEqual,
                .depthBoundsTestEnable = false,
                .stencilTestEnable = false,
                .front = vk::StencilOpState{.failOp = vk::StencilOp::eKeep,.passOp = vk::StencilOp::eKeep, .depthFailOp = vk::StencilOp::eKeep, .compareOp = vk::CompareOp::eAlways },
                .back = vk::StencilOpState{.failOp = vk::StencilOp::eKeep,.passOp = vk::StencilOp::eKeep, .depthFailOp = vk::StencilOp::eKeep, .compareOp = vk::CompareOp::eAlways }
            },
            .multisampling_state_create_info = vk::PipelineMultisampleStateCreateInfo
            {
                .rasterizationSamples = vk::SampleCountFlagBits::e1,
                .pSampleMask = nullptr
            }
        };
    }

    MeshPipelinePreset CreateMeshPipelinePreset()
    {
        MeshPipelinePreset meshPipelinePreset
        {
            .color_blend_attachment_state = vk::PipelineColorBlendAttachmentState
            {
                .blendEnable = false,
                .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
            },
            .enabled_dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor }
        };

        return meshPipelinePreset;
    }

    PerFrameGeneralPurposeDescriptorSetLayout::PerFrameGeneralPurposeDescriptorSetLayout(DeviceContext& deviceContext)
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

    ColoredVertexMeshPipeline::ColoredVertexMeshPipeline(
        DeviceContext& deviceContext, 
        std::shared_ptr<DepthColorRenderPass> renderPass,
        std::shared_ptr<PerFrameGeneralPurposeDescriptorSetLayout> descriptorSetLayout
    ) : PipelineBase(std::move(renderPass), std::move(descriptorSetLayout))
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

        auto preset = CreateMeshPipelinePreset();
        SetupFixedPreset(preset);

        //
        // Shaders
        // 

        auto vshader = deviceContext.Shaders()->LoadShaderModule(ColoredVertexVS);
        auto fshader = deviceContext.Shaders()->LoadShaderModule(ColoredVertexFS);

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
            .pInputAssemblyState = &preset.fixed_preset.input_assembly_create_info,
            .pTessellationState = nullptr,
            .pViewportState = &preset.fixed_preset.viewport_state_create_info,
            .pRasterizationState = &preset.fixed_preset.rasterization_state_create_info,
            .pMultisampleState = &preset.fixed_preset.multisampling_state_create_info,
            .pDepthStencilState = &preset.fixed_preset.depth_stencil_state_create_info,
            .pColorBlendState = &preset.fixed_preset.color_blend_state_create_info,
            .pDynamicState = &preset.fixed_preset.dynamic_state_create_info,
            .layout = *_pipelineLayout,
            .renderPass = _renderPass->Get()
        };

        _pipeline = deviceContext.LogicalDevice()->createGraphicsPipeline(
            deviceContext.Shaders()->Cache(),
            pipelineCreateInfo
        );
    }



    DescriptorPool::DescriptorPool(DeviceContext& deviceContext)
        : _device(deviceContext.LogicalDevice())
    {
        // We need to tell the API the number of max. requested descriptors per type
        std::array<vk::DescriptorPoolSize, 1> perTypeMaxCount{};
        perTypeMaxCount[0] = vk::DescriptorPoolSize{ .type = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1 };

        // For additional types you need to add new entries in the type count list
        // E.g. for two combined image samplers :
        // typeCounts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        // typeCounts[1].descriptorCount = 2;

        // Create the global descriptor pool
        // All descriptors used in this example are allocated from this pool
        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo
        {
            .maxSets = 1,
            .poolSizeCount = static_cast<uint32_t>(perTypeMaxCount.size()),
            .pPoolSizes = perTypeMaxCount.data()
        };
        _pool = _device->createDescriptorPool(descriptorPoolCreateInfo);
    }

}