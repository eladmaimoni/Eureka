#include "Pipeline.hpp"
#include "PipelineTypes.hpp"

#include "PipelineHelpers.hpp"
#include "ShaderReflection.hpp"

namespace eureka
{
    //////////////////////////////////////////////////////////////////////////
    //
    //                        UIPipeline
    // 
    //////////////////////////////////////////////////////////////////////////


    ImGuiPipeline::ImGuiPipeline(
        DeviceContext& deviceContext,
        DescriptorSetLayoutCache& layoutCache,
        vk::RenderPass renderPass
    ) : _fragmentShaderSetLayout(layoutCache.RetrieveLayout(0, SET0_ID_001_PER_FONT))
    {
        std::array<vk::DescriptorSetLayout, 1> setLayouts
        {
            _fragmentShaderSetLayout,
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

        Setup(deviceContext, renderPass);
    }



    void ImGuiPipeline::Setup(DeviceContext& deviceContext, vk::RenderPass renderPass)
    {
        //
        // Vertex Attributes 
        //

        auto layout = MakeInterleavedVertexLayout<ImGuiVertex>();

        vk::PipelineVertexInputStateCreateInfo vertexInputState
        {
            .vertexBindingDescriptionCount = static_cast<uint32_t>(layout.bindings.size()),
            .pVertexBindingDescriptions = layout.bindings.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(layout.attributes.size()),
            .pVertexAttributeDescriptions = layout.attributes.data()
        };

        DefaultFixedPiplinePreset preset{};
        SetupFixedPreset<UIFixedPresetTraits>(preset);

        std::array<ShaderId, 2> ids{ ImGuiVS, ImGuiFS };
        auto shadersPipeline = MakeShaderPipeline(ids, * deviceContext.Shaders());


        //auto _pipelineLayout2 = ReflectPipeline(dynamic_span<ShaderId>(ids.data(), ids.size()));
        //
        // All Together
        //

        vk::GraphicsPipelineCreateInfo pipelineCreateInfo
        {
            .stageCount = static_cast<uint32_t>(shadersPipeline.stages.size()),
            .pStages = shadersPipeline.stages.data(),
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
        DescriptorSetLayoutCache& layoutCache,
        vk::RenderPass renderPass
    ) 
    {
        _descLayout = layoutCache.RetrieveLayout(0, SET0_ID_000_PER_VIEW);
       
        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo
        {
            .setLayoutCount = 1,
            .pSetLayouts = &_descLayout
        };

        _pipelineLayout = deviceContext.LogicalDevice()->createPipelineLayout(pipelineLayoutCreateInfo);

 

        Setup(deviceContext, renderPass);
    }

    void ColoredVertexMeshPipeline::Setup(DeviceContext& deviceContext, vk::RenderPass renderPass)
    {
        //
        // Vertex Attributes 
        //

        auto layout = MakeInterleavedVertexLayout<PositionColorVertex>();

        vk::PipelineVertexInputStateCreateInfo vertexInputState
        {
            .vertexBindingDescriptionCount = static_cast<uint32_t>(layout.bindings.size()),
            .pVertexBindingDescriptions = layout.bindings.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(layout.attributes.size()),
            .pVertexAttributeDescriptions = layout.attributes.data()
        };

        DefaultFixedPiplinePreset preset{};
        SetupFixedPreset<MeshFixedPresetTraits>(preset);

        std::array<ShaderId, 2> ids{ ColoredVertexVS, ColoredVertexFS };
        auto shadersPipeline = MakeShaderPipeline(ids, *deviceContext.Shaders());

      

        //
        // All Together
        //

        vk::GraphicsPipelineCreateInfo pipelineCreateInfo
        {
            .stageCount = static_cast<uint32_t>(shadersPipeline.stages.size()),
            .pStages = shadersPipeline.stages.data(),
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
        vk::RenderPass renderPass,
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

        Setup(deviceContext, renderPass);
    }

    void PhongShadedMeshWithNormalMapPipeline::Setup(DeviceContext& deviceContext, vk::RenderPass renderPass)
    {
        //
        // Vertex Attributes 
        //

        auto layout = MakeDeinterleavedVertexLayout<Eigen::Vector3f, Eigen::Vector3f, Eigen::Vector2f, Eigen::Vector4f>();

        vk::PipelineVertexInputStateCreateInfo vertexInputState
        {
            .vertexBindingDescriptionCount = static_cast<uint32_t>(layout.bindings.size()),
            .pVertexBindingDescriptions = layout.bindings.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(layout.attributes.size()),
            .pVertexAttributeDescriptions = layout.attributes.data()
        };

        DefaultFixedPiplinePreset preset{};
        SetupFixedPreset<MeshFixedPresetTraits>(preset);


        auto shadersPipeline = MakeShaderPipeline(std::array<ShaderId, 2> { ShadedMeshWithNormalMapVS, PhongShadedMeshWithNormalMapFS }, * deviceContext.Shaders());

        //
        // All Together
        //

        vk::GraphicsPipelineCreateInfo pipelineCreateInfo
        {
            .stageCount = static_cast<uint32_t>(shadersPipeline.stages.size()),
            .pStages = shadersPipeline.stages.data(),
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