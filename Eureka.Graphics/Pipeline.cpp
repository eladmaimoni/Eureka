#include "Pipeline.hpp"
#include "PipelineTypes.hpp"
#include <ShadersCache.hpp>
#include "PipelineHelpers.hpp"

namespace eureka
{
    //////////////////////////////////////////////////////////////////////////
    //
    //                        UIPipeline
    // 
    //////////////////////////////////////////////////////////////////////////


    ImGuiPipeline::ImGuiPipeline(
        DeviceContext& deviceContext,
        const DepthColorRenderPass& renderPass,
        const SingleFragmentShaderCombinedImageSamplerDescriptorSetLayout& fragmentShaderSetLayout
    )
    {
        std::array<vk::DescriptorSetLayout, 1> setLayouts
        {
            fragmentShaderSetLayout.Get(),
        };

        vk::PushConstantRange pushConstantsRange
        {
            .stageFlags = vk::ShaderStageFlagBits::eVertex,
            .offset = 0,
            .size = sizeof(ImGuiPushConstantsBlock)
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

        DefaultFixedPiplinePreset preset{};
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
            .pInputAssemblyState = &preset.input_assembly_create_info,
            .pTessellationState = nullptr,
            .pViewportState = &preset.viewport_state_create_info,
            .pRasterizationState = &preset.rasterization_state_create_info,
            .pMultisampleState = &preset.multisampling_state_create_info,
            .pDepthStencilState = &preset.depth_stencil_state_create_info,
            .pColorBlendState = &preset.color_blend_state_create_info,
            .pDynamicState = &preset.dynamic_state_create_info,
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

        DefaultFixedPiplinePreset preset{};
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
            .pInputAssemblyState = &preset.input_assembly_create_info,
            .pTessellationState = nullptr,
            .pViewportState = &preset.viewport_state_create_info,
            .pRasterizationState = &preset.rasterization_state_create_info,
            .pMultisampleState = &preset.multisampling_state_create_info,
            .pDepthStencilState = &preset.depth_stencil_state_create_info,
            .pColorBlendState = &preset.color_blend_state_create_info,
            .pDynamicState = &preset.dynamic_state_create_info,
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
        const ColorAndNormalMapFragmentDescriptorSetLayout& perNormalMappedModelDescriptorSetLayout
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

        DefaultFixedPiplinePreset preset{};
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
            .pInputAssemblyState = &preset.input_assembly_create_info,
            .pTessellationState = nullptr,
            .pViewportState = &preset.viewport_state_create_info,
            .pRasterizationState = &preset.rasterization_state_create_info,
            .pMultisampleState = &preset.multisampling_state_create_info,
            .pDepthStencilState = &preset.depth_stencil_state_create_info,
            .pColorBlendState = &preset.color_blend_state_create_info,
            .pDynamicState = &preset.dynamic_state_create_info,
            .layout = *_pipelineLayout,
            .renderPass = renderPass
        };

        _pipeline = deviceContext.LogicalDevice()->createGraphicsPipeline(
            deviceContext.Shaders()->Cache(),
            pipelineCreateInfo
        );
    }



}