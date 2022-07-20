#pragma once
#include "PipelineTypesReflection.hpp"
#include <ShadersCache.hpp>
#include <boost/hana/for_each.hpp>
//#include <boost/hana/members.hpp>
//#include <boost/hana/size.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace eureka
{
    struct SingleColorOutputAttachmentPreset
    {
        std::array<vk::PipelineColorBlendAttachmentState, 1> color_blend_attachment_state
        {
            vk::PipelineColorBlendAttachmentState
            {
                .blendEnable = true,
                .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
                .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
                .colorBlendOp = vk::BlendOp::eAdd,
                .srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
                .dstAlphaBlendFactor = vk::BlendFactor::eZero,
                .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
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



    //////////////////////////////////////////////////////////////////////////
    //
    //                        Vertex Layout       
    //
    //////////////////////////////////////////////////////////////////////////
   
    template<std::size_t BindingCount, std::size_t AttributeCount>
    struct VertexLayout
    {
        std::array<vk::VertexInputBindingDescription, BindingCount>     bindings;
        std::array<vk::VertexInputAttributeDescription, AttributeCount> attributes;
    };

    struct VertexLayoutTraits
    {
        static constexpr bool is_interleaved = false;
    };

    template<typename AttributeType>
    struct AttributeFormat
    {
        //static constexpr vk::Format format = vk::Format::eUndefined;

    };
    template<>
    struct AttributeFormat<uint32_t>
    {
        static constexpr vk::Format format = vk::Format::eR8G8B8A8Unorm;
    };

    template<>
    struct AttributeFormat<Eigen::Vector2f>
    {
        static constexpr vk::Format format = vk::Format::eR32G32Sfloat;
    };
    template<>
    struct AttributeFormat<Eigen::Vector3f> 
    {
        static constexpr vk::Format format = vk::Format::eR32G32B32Sfloat; 
    };

    template<>
    struct AttributeFormat<Eigen::Vector4f>
    {
        static constexpr vk::Format format = vk::Format::eR32G32B32A32Sfloat;
    };

    template<typename ... Attributes>
    auto MakeDeinterleavedVertexLayout()
    {
        constexpr std::size_t AttributeCount = sizeof...(Attributes);

        VertexLayout< AttributeCount, AttributeCount> layout{};
        std::array<vk::VertexInputAttributeDescription, AttributeCount> vertexInputAttributs;

        uint32_t i = 0;

        ([&]
            {
                layout.bindings[i] =
                    vk::VertexInputBindingDescription
                {
                    .binding = i,
                    .stride = sizeof(Attributes),
                    .inputRate = vk::VertexInputRate::eVertex
                };

                layout.attributes[i] =
                    vk::VertexInputAttributeDescription
                {
                    .location = i,
                    .binding = i,
                    .format = AttributeFormat<Attributes>::format,
                    .offset = 0
                };
                ++i;
            }
         (), ...);

        return layout;
    }

    template<typename T>
    constexpr std::size_t CountFields()
    {
        namespace hana = boost::hana;
        std::size_t count = 0;

        hana::for_each(hana::accessors<T>(), [&](auto )
            {
                ++count;
            });

        return count;
    }

    template<typename VertexStruct>
    auto MakeInterleavedVertexLayout()
    {
        VertexStruct value{}; // dummy, TODO maybe we can somehow remove this
        namespace hana = boost::hana;
        constexpr std::size_t AttributeCount = CountFields<VertexStruct>();

        VertexLayout<1, AttributeCount> layout{};
        std::array<vk::VertexInputAttributeDescription, AttributeCount> vertexInputAttributs;

        layout.bindings[0] =
            vk::VertexInputBindingDescription
        {
            .binding = 0,
            .stride = sizeof(VertexStruct),
            .inputRate = vk::VertexInputRate::eVertex
        };

        uint32_t i = 0;

        hana::for_each(hana::accessors<VertexStruct>(), [&](auto pair)
            {
                auto member_access = hana::second(pair);
                using value_t = std::decay_t<decltype(member_access(value))>;
                auto offset = reinterpret_cast<uint64_t>(&member_access(value)) - reinterpret_cast<uint64_t>(&value);
                layout.attributes[i] =
                    vk::VertexInputAttributeDescription
                {
                    .location = i,
                    .binding = 0,
                    .format = AttributeFormat<value_t>::format,
                    .offset = static_cast<uint32_t>(offset)
                };
                ++i;
            });
        return layout;
    }


    template<std::size_t COUNT>
    struct ShadersPipeline
    {
        svec5<vkr::ShaderModule> modules;
        //std::array<vkr::ShaderModule, COUNT> modules{ nullptr, nullptr }; // annoying vulkan raii
        std::array<vk::PipelineShaderStageCreateInfo, COUNT> stages;
    };

    template<std::size_t COUNT>
    auto MakeShaderPipeline(std::array<ShaderId, COUNT> ids, ShaderCache& shaderCache)
    {
        ShadersPipeline<COUNT> shaderPipeline;

        for (auto i = 0u; i < ids.size(); ++i)
        {
            //shaderPipeline.modules[i] = shaderCache.LoadShaderModule(ids[i]);
            shaderPipeline.modules.emplace_back(shaderCache.LoadShaderModule(ids[i]));
            shaderPipeline.stages[i] = vk::PipelineShaderStageCreateInfo
            {
                .stage = ids[i].shader_type,
                .module = *shaderPipeline.modules[i],
                .pName = "main" 
            };
        }
        return shaderPipeline;
    }
}

