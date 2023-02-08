#include "PipelinePresets.hpp"
#include "PipelineHelpers.hpp"

namespace eureka::vulkan
{
    PipelineLayoutCreationPreset::PipelineLayoutCreationPreset(PipelinePresetType preset, const DescriptorSetLayoutCache& layoutCache)
    {
        if (preset == PipelinePresetType::eImGui || preset == PipelinePresetType::eTexturedRegion)
        {
            _setLayoutHandles.emplace_back(layoutCache.GetLayoutHandle(DescriptorSet0PresetType::eSingleTexture));

            VkPushConstantRange pushConstantsRange{
                .stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0,
                .size = sizeof(ScaleTranslatePushConstantsBlock),
            };
            _pushConstantRanges.emplace_back(pushConstantsRange);

            _createInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            _createInfo.setLayoutCount = static_cast<uint32_t>(_setLayoutHandles.size());
            _createInfo.pSetLayouts = _setLayoutHandles.data();
            _createInfo.pushConstantRangeCount = static_cast<uint32_t>(_pushConstantRanges.size());
            _createInfo.pPushConstantRanges = _pushConstantRanges.data();
        }
        else
        {
            throw std::logic_error("bad");
        }
    }


    // MUST BE NO MOVE NO COPY

    PipelineCreationPreset::PipelineCreationPreset(PipelinePresetType preset, ShaderCache& shaderCache, VkPipelineLayout layoutHandle, VkRenderPass renderPassHandle)
    {
        if (preset == PipelinePresetType::eImGui)
        {
            _vertexLayout = MakeInterleavedVertexLayout<ImGuiVertex>();
            _stages = MakeShaderPipeline(std::array<ShaderId, 2> { ImGuiVS, ImGuiFS }, shaderCache);

        }
        else if (preset == PipelinePresetType::eTexturedRegion)
        {
            // no vertex data
            _stages = MakeShaderPipeline(std::array<ShaderId, 2> { Textured2DRegionVS, Textured2DRegionFS }, shaderCache);
        }
        else
        {
            throw std::logic_error("bad");
        }


        _vertexInput.vertexBindingDescriptionCount = static_cast<uint32_t>(_vertexLayout.bindings.size());
        _vertexInput.pVertexBindingDescriptions = _vertexLayout.bindings.data();
        _vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(_vertexLayout.attributes.size());
        _vertexInput.pVertexAttributeDescriptions = _vertexLayout.attributes.data();
        _dynamic.dynamicStateCount = static_cast<uint32_t>(_dynamicStates.size());
        _dynamic.pDynamicStates = _dynamicStates.data();
        _colorBlend.attachmentCount = static_cast<uint32_t>(_colorBlendAttachments.size());
        _colorBlend.pAttachments = _colorBlendAttachments.data();
        _createInfo.stageCount = static_cast<uint32_t>(_stages.stages.size());
        _createInfo.pStages = _stages.stages.data();

        _createInfo.pVertexInputState = &_vertexInput;
        _createInfo.pInputAssemblyState = &_inputAssembly;
        _createInfo.pTessellationState = nullptr;
        _createInfo.pViewportState = &_viewport;
        _createInfo.pRasterizationState = &_rasterization;
        _createInfo.pMultisampleState = &_multisampling;
        _createInfo.pDepthStencilState = &_depthStencil;
        _createInfo.pColorBlendState = &_colorBlend;
        _createInfo.pDynamicState = &_dynamic;
        _createInfo.layout = layoutHandle;
        _createInfo.renderPass = renderPassHandle;
    }

}

