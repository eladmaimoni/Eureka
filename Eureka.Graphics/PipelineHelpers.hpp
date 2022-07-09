#pragma once

namespace eureka
{
    struct SingleColorOutputAttachmentPreset
    {
        std::array<vk::PipelineColorBlendAttachmentState, 1> color_blend_attachment_state
        { 
            vk::PipelineColorBlendAttachmentState
            {
                .blendEnable = false,
                .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
            }
        };
    };

    struct DynamicViewprtScissorPreset
    {
        std::array<vk::DynamicState, 2> enabled_dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    };

    template<typename ColorBlendAttachmentPreset, typename DynamicStatePreset>
    struct FixedPiplinePreset
    {
        vk::PipelineInputAssemblyStateCreateInfo input_assembly_create_info;
        vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info;
        vk::PipelineColorBlendStateCreateInfo    color_blend_state_create_info;
        vk::PipelineViewportStateCreateInfo      viewport_state_create_info;
        vk::PipelineDynamicStateCreateInfo       dynamic_state_create_info;
        vk::PipelineDepthStencilStateCreateInfo  depth_stencil_state_create_info;
        vk::PipelineMultisampleStateCreateInfo   multisampling_state_create_info;
        ColorBlendAttachmentPreset               color_blend_preset;
        DynamicStatePreset                       dynamic_state_preset;
    };

    using DefaultFixedPiplinePreset = FixedPiplinePreset<SingleColorOutputAttachmentPreset, DynamicViewprtScissorPreset>;

    //struct MeshPipelinePreset
    //{
    //    vk::PipelineColorBlendAttachmentState color_blend_attachment_state;
    //    std::array<vk::DynamicState, 2>       enabled_dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    //    FixedPiplinePreset                    fixed_preset;
    //};

    //////////////////////////////////////////////////////////////////////////
    //
    //                        FixedPresetTraits       
    //
    //////////////////////////////////////////////////////////////////////////
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

    //////////////////////////////////////////////////////////////////////////
    //
    //                        SetupFixedPreset       
    //
    //////////////////////////////////////////////////////////////////////////
    template< typename Traits, typename FixedPreset>
    void SetupFixedPreset(FixedPreset& meshPipelinePreset)
    {
        meshPipelinePreset.input_assembly_create_info = vk::PipelineInputAssemblyStateCreateInfo
        {
            .topology = vk::PrimitiveTopology::eTriangleList
        };
        meshPipelinePreset.rasterization_state_create_info = vk::PipelineRasterizationStateCreateInfo
        {
            .depthClampEnable = false,
            .rasterizerDiscardEnable = false,
            .polygonMode = vk::PolygonMode::eFill,
            .cullMode = Traits::cull_mode,
            .frontFace = vk::FrontFace::eCounterClockwise,
            .depthBiasEnable = false,
            .lineWidth = 1.0f
        };
        meshPipelinePreset.color_blend_state_create_info = vk::PipelineColorBlendStateCreateInfo
        {
            .attachmentCount = static_cast<uint32_t>(meshPipelinePreset.color_blend_preset.color_blend_attachment_state.size()),
            .pAttachments = meshPipelinePreset.color_blend_preset.color_blend_attachment_state.data()
        };
        meshPipelinePreset.viewport_state_create_info = vk::PipelineViewportStateCreateInfo
        {
            .viewportCount = 1,
            .scissorCount = 1
        };
        meshPipelinePreset.dynamic_state_create_info = vk::PipelineDynamicStateCreateInfo
        {
            .dynamicStateCount = static_cast<uint32_t>(meshPipelinePreset.dynamic_state_preset.enabled_dynamic_states.size()),
            .pDynamicStates = meshPipelinePreset.dynamic_state_preset.enabled_dynamic_states.data()
        };
        meshPipelinePreset.depth_stencil_state_create_info = vk::PipelineDepthStencilStateCreateInfo
        {
            .depthTestEnable = Traits::depth_test,
            .depthWriteEnable = Traits::depth_write,
            .depthCompareOp = vk::CompareOp::eLessOrEqual,
            .depthBoundsTestEnable = false,
            .stencilTestEnable = false,
            .front = vk::StencilOpState{.failOp = vk::StencilOp::eKeep,.passOp = vk::StencilOp::eKeep, .depthFailOp = vk::StencilOp::eKeep, .compareOp = vk::CompareOp::eAlways },
            .back = vk::StencilOpState{.failOp = vk::StencilOp::eKeep,.passOp = vk::StencilOp::eKeep, .depthFailOp = vk::StencilOp::eKeep, .compareOp = vk::CompareOp::eAlways }
        };
        meshPipelinePreset.multisampling_state_create_info = vk::PipelineMultisampleStateCreateInfo
        {
            .rasterizationSamples = vk::SampleCountFlagBits::e1,
            .pSampleMask = nullptr
        };
        
    }


    //inline MeshPipelinePreset CreateMeshPipelinePreset()
    //{
    //    MeshPipelinePreset meshPipelinePreset
    //    {
    //        .color_blend_attachment_state = vk::PipelineColorBlendAttachmentState
    //        {
    //            .blendEnable = false,
    //            .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    //        },
    //        .enabled_dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor }
    //    };

    //    return meshPipelinePreset;
    //}
}

