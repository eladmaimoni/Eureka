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

    struct MeshFixedPresetTraits
    {
        static constexpr vk::CullModeFlags cull_mode = vk::CullModeFlagBits::eNone;
        static constexpr bool depth_test = true;
        static constexpr bool depth_write = true;
    };
    struct UIFixedPresetTraits
    {
        static constexpr vk::CullModeFlags cull_mode = vk::CullModeFlagBits::eNone;
        static constexpr bool depth_test = false;
        static constexpr bool depth_write = false;
    };
    template<typename Traits>
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
                .cullMode = Traits::cull_mode,
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
                .depthTestEnable = Traits::depth_test,
                .depthWriteEnable = Traits::depth_write,
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

    //////////////////////////////////////////////////////////////////////////
    //
    //                        UIPipeline
    // 
    //////////////////////////////////////////////////////////////////////////

    void ImGuiPipeline::Setup(DeviceContext& deviceContext, vk::RenderPass renderPass)
    {
        //
        // Vertex Attributes 
        //
        
        vk::VertexInputBindingDescription vertexInputBinding
        {
            .binding = 0,
            .stride = sizeof(ImGuiVertex),
            .inputRate = vk::VertexInputRate::eVertex
        };

        std::array<vk::VertexInputAttributeDescription, 3> vertexInputAttributs
        {
            // xyz position at shader location 0
            vk::VertexInputAttributeDescription
            {
                .location = 0,
                .binding = 0,
                .format = vk::Format::eR32G32Sfloat,
                .offset = offsetof(ImGuiVertex, position)
            },
            vk::VertexInputAttributeDescription
            {
                .location = 1,
                .binding = 0,
                .format = vk::Format::eR32G32Sfloat,
                .offset = offsetof(ImGuiVertex, uv)
            },
            // rgb color at shader location 1
            vk::VertexInputAttributeDescription
            {
                .location = 2,
                .binding = 0,
                .format = vk::Format::eR8G8B8Unorm,
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
        SetupFixedPreset<UIFixedPresetTraits>(preset);

        //
        // Shaders
        // 

        auto vshader = deviceContext.Shaders()->LoadShaderModule(ImGuiVS);
        auto fshader = deviceContext.Shaders()->LoadShaderModule(ImGuiFS);

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
            .renderPass = renderPass
        };

        _pipeline = deviceContext.LogicalDevice()->createGraphicsPipeline(
            deviceContext.Shaders()->Cache(),
            pipelineCreateInfo
        );
    }

    ImGuiPipeline::ImGuiPipeline(DeviceContext& deviceContext, const DepthColorRenderPass& renderPass, const SingleVertexShaderUBODescriptorSetLayout& descriptorSetLayout)
    {
        _descLayout = descriptorSetLayout.Get();
        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo
        {
            .setLayoutCount = 1,
            .pSetLayouts = &_descLayout
        };

        _pipelineLayout = deviceContext.LogicalDevice()->createPipelineLayout(pipelineLayoutCreateInfo);

        Setup(deviceContext, renderPass.Get());
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //                        ColoredVertexMeshPipeline
    // 
    //////////////////////////////////////////////////////////////////////////
    ColoredVertexMeshPipeline::ColoredVertexMeshPipeline(
        DeviceContext& deviceContext, 
        const DepthColorRenderPass& renderPass,
        const SingleVertexShaderUBODescriptorSetLayout& descriptorSetLayout
    ) 
    {
        _descLayout = descriptorSetLayout.Get();
        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo
        {
            .setLayoutCount = 1,
            .pSetLayouts = &_descLayout
        };

        _pipelineLayout = deviceContext.LogicalDevice()->createPipelineLayout(pipelineLayoutCreateInfo);

        Setup(deviceContext, renderPass.Get());
    }

    void ColoredVertexMeshPipeline::Setup(DeviceContext& deviceContext, vk::RenderPass renderPass)
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
        SetupFixedPreset<MeshFixedPresetTraits>(preset);

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
            .renderPass = renderPass
        };

        _pipeline = deviceContext.LogicalDevice()->createGraphicsPipeline(
            deviceContext.Shaders()->Cache(),
            pipelineCreateInfo
        );
    }



    //////////////////////////////////////////////////////////////////////////
    //
    //                    NormalMappedShadedMeshPipeline
    // 
    //////////////////////////////////////////////////////////////////////////


    PhongShadedMeshWithNormalMapPipeline::PhongShadedMeshWithNormalMapPipeline(
        DeviceContext& deviceContext, 
        const DepthColorRenderPass& renderPass,
        const SingleVertexShaderUBODescriptorSetLayout& perViewDescriptorSetLayout,
        const PerNormalMappedModelDescriptorSetLayout& perNormalMappedModelDescriptorSetLayout
    )
    {
   

        std::array<vk::DescriptorSetLayout, 2> setLayouts
        {
            perViewDescriptorSetLayout.Get(),
            perNormalMappedModelDescriptorSetLayout.Get(),
        };

        vk::PushConstantRange pushConstantsRange
        {
            .stageFlags = vk::ShaderStageFlagBits::eVertex,
            .offset = 0,
            .size = sizeof(Eigen::Matrix4f)
        };

        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo
        {
            .setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
            .pSetLayouts = setLayouts.data(),
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &pushConstantsRange
        };

        _pipelineLayout = deviceContext.LogicalDevice()->createPipelineLayout(pipelineLayoutCreateInfo);

        Setup(deviceContext, renderPass.Get());
    }

    void PhongShadedMeshWithNormalMapPipeline::Setup(DeviceContext& deviceContext, vk::RenderPass renderPass)
    {
        //
        // Vertex Attributes 
        //

        std::array<vk::VertexInputBindingDescription, 4> vertexInputBindings
        {
            vk::VertexInputBindingDescription
            {
                .binding = 0,
                .stride = sizeof(Eigen::Vector3f),
                .inputRate = vk::VertexInputRate::eVertex
            },
                vk::VertexInputBindingDescription
            {
                .binding = 1,
                .stride = sizeof(Eigen::Vector3f),
                .inputRate = vk::VertexInputRate::eVertex
            },
                vk::VertexInputBindingDescription
            {
                .binding = 2,
                .stride = sizeof(Eigen::Vector2f),
                .inputRate = vk::VertexInputRate::eVertex
            },
                vk::VertexInputBindingDescription
            {
                .binding = 3,
                .stride = sizeof(Eigen::Vector3f),
                .inputRate = vk::VertexInputRate::eVertex
            }
        };


        std::array<vk::VertexInputAttributeDescription, 4> vertexInputAttributs
        {

            vk::VertexInputAttributeDescription
            { // xyz position at shader location 0
                .location = 0,
                .binding = 0,
                .format = vk::Format::eR32G32B32Sfloat,
                .offset = 0
            },
            vk::VertexInputAttributeDescription
            { // normal at shader location 1
                .location = 1,
                .binding = 1,
                .format = vk::Format::eR32G32B32Sfloat,
                .offset = 0
            },
                        vk::VertexInputAttributeDescription
            {
                // uv at shader location 2
                .location = 2,
                .binding = 2,
                .format = vk::Format::eR32G32Sfloat,
                .offset = 0
            },
            // rgb color at shader location 1
            vk::VertexInputAttributeDescription
            {
                // tangent at shader location 3
                .location = 3,
                .binding = 3,
                .format = vk::Format::eR32G32B32Sfloat,
                .offset = 0
            }
        };

        vk::PipelineVertexInputStateCreateInfo vertexInputState
        {
            .vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size()),
            .pVertexBindingDescriptions = vertexInputBindings.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributs.size()),
            .pVertexAttributeDescriptions = vertexInputAttributs.data()
        };

        auto preset = CreateMeshPipelinePreset();
        SetupFixedPreset<MeshFixedPresetTraits>(preset);

        //
        // Shaders
        // 

        auto vshader = deviceContext.Shaders()->LoadShaderModule(ShadedMeshWithNormalMapVS);
        auto fshader = deviceContext.Shaders()->LoadShaderModule(PhongShadedMeshWithNormalMapFS);

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
            .renderPass = renderPass
        };

        _pipeline = deviceContext.LogicalDevice()->createGraphicsPipeline(
            deviceContext.Shaders()->Cache(),
            pipelineCreateInfo
        );
    }



}