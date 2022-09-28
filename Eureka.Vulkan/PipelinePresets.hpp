#pragma once
#include "DescriptorLayoutCache.hpp"
#include "PipelineTypes.hpp"
#include "ShaderModule.hpp"
#include "ShadersCache.hpp"
#include <vector>
#include <macros.hpp>

namespace eureka::vulkan
{

} // namespace eureka::vulkan

namespace eureka::vulkan
{

    enum class PipelinePresetType
    {
        eImGui
    };

    class PipelineLayoutCreationPreset
    {
        svec2<VkPushConstantRange>   _pushConstantRanges;
        svec3<VkDescriptorSetLayout> _setLayoutHandles;
        VkPipelineLayoutCreateInfo   _createInfo {};

    public:
        const VkPipelineLayoutCreateInfo& GetCreateInfo() const
        {
            return _createInfo;
        }

        PipelineLayoutCreationPreset(PipelinePresetType preset, const DescriptorSetLayoutCache& layoutCache);
        PipelineLayoutCreationPreset(PipelineLayoutCreationPreset&& that) = default;
        PipelineLayoutCreationPreset& operator=(PipelineLayoutCreationPreset&& rhs) = default;
        PipelineLayoutCreationPreset(const PipelineLayoutCreationPreset& that) = delete;
        PipelineLayoutCreationPreset& operator=(const PipelineLayoutCreationPreset& rhs) = delete;
        ~PipelineLayoutCreationPreset() = default;
    };

    inline const VkStencilOpState DEFAULT_STENCIL_OP {
        .failOp = VkStencilOp::VK_STENCIL_OP_KEEP,
        .passOp = VkStencilOp::VK_STENCIL_OP_KEEP,
        .depthFailOp = VkStencilOp::VK_STENCIL_OP_KEEP,
        .compareOp = VkCompareOp::VK_COMPARE_OP_ALWAYS,
    };

    inline const VkPipelineVertexInputStateCreateInfo DEFAULT_VERTEX_INPUT_STATE {
        .sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    };

    inline const VkPipelineInputAssemblyStateCreateInfo DEFAULT_INPUT_ASSEMBLY_STATE {
        .sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    };

    inline const VkPipelineRasterizationStateCreateInfo DEFAULT_RASTERIZATION_STATE {
        .sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = false,
        .rasterizerDiscardEnable = false,
        .polygonMode = VkPolygonMode::VK_POLYGON_MODE_FILL,
        .cullMode = VkCullModeFlagBits::VK_CULL_MODE_NONE,
        .frontFace = VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = false,
        .lineWidth = 1.0f,
    };

    inline const VkPipelineColorBlendStateCreateInfo DEFAULT_COLOR_BLEND_STATE {
        .sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    };

    inline const VkPipelineViewportStateCreateInfo DEFAULT_VIEWPORT_STATE {
        .sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    inline const VkPipelineDynamicStateCreateInfo DEFAULT_DYNAMIC_STATE {
        .sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    };

    inline const VkPipelineDepthStencilStateCreateInfo DEFAULT_DEPTH_STENCIL_STATE {
        .sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    };

    inline const VkPipelineMultisampleStateCreateInfo DEFAULT_MULTISAMPLING_STATE {
        .sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
    };

    inline const VkPipelineColorBlendAttachmentState DEFAULT_COLOR_BLEND_ATTACHMENT_STATE {
        .blendEnable = true,
        .srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO,
        .colorWriteMask =
            VkColorComponentFlagBits::VK_COLOR_COMPONENT_R_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_G_BIT |
            VkColorComponentFlagBits::VK_COLOR_COMPONENT_B_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_A_BIT,
    };

    class PipelineCreationPreset
    {
        VertexLayout                               _vertexLayout;
        ShadersPipeline                            _stages;
        svec2<VkPipelineColorBlendAttachmentState> _colorBlendAttachments {DEFAULT_COLOR_BLEND_ATTACHMENT_STATE};
        svec2<VkDynamicState>                      _dynamicStates {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineVertexInputStateCreateInfo       _vertexInput = DEFAULT_VERTEX_INPUT_STATE;
        VkPipelineInputAssemblyStateCreateInfo     _inputAssembly = DEFAULT_INPUT_ASSEMBLY_STATE;
        VkPipelineRasterizationStateCreateInfo     _rasterization = DEFAULT_RASTERIZATION_STATE;
        VkPipelineColorBlendStateCreateInfo        _colorBlend = DEFAULT_COLOR_BLEND_STATE;
        VkPipelineViewportStateCreateInfo          _viewport = DEFAULT_VIEWPORT_STATE;
        VkPipelineDynamicStateCreateInfo           _dynamic = DEFAULT_DYNAMIC_STATE;
        VkPipelineDepthStencilStateCreateInfo      _depthStencil = DEFAULT_DEPTH_STENCIL_STATE;
        VkPipelineMultisampleStateCreateInfo       _multisampling = DEFAULT_MULTISAMPLING_STATE;

        VkGraphicsPipelineCreateInfo _createInfo {.sType = VkStructureType::VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,};

    public:
        EUREKA_NO_COPY_NO_MOVE(PipelineCreationPreset);
        // MUST BE NO MOVE NO COPY
        PipelineCreationPreset(PipelinePresetType preset,
                               ShaderCache&       shaderCache,
                               VkPipelineLayout   layoutHandle,
            VkRenderPass       renderPassHandle);

        const VkGraphicsPipelineCreateInfo& GetCreateInfo() const
        {
            return _createInfo;
        }
    };

} // namespace eureka::vulkan
