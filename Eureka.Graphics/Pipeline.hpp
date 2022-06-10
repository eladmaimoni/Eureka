#pragma once
#include "DeviceContext.hpp"
#include "VkHelpers.hpp"
#include "vk_error_handling.hpp"
#include "RenderPass.hpp"
#include "Mesh.hpp"

namespace eureka
{       
    //////////////////////////////////////////////////////////////////////////
    //
    // This is the set of constant buffers the shader receives
    // the shader expects a single set of descriptors with only one entry ('binding')
    // 
    //////////////////////////////////////////////////////////////////////////
    class PerFrameGeneralPurposeDescriptorSet
    {
        vkr::DescriptorSetLayout _descriptorSetLayout{ nullptr };       
    public:
        vk::DescriptorSetLayout Get() const { return *_descriptorSetLayout; }
        PerFrameGeneralPurposeDescriptorSet() = default;
        PerFrameGeneralPurposeDescriptorSet(PerFrameGeneralPurposeDescriptorSet&& that) = default;
        PerFrameGeneralPurposeDescriptorSet& operator=(PerFrameGeneralPurposeDescriptorSet&& that) = default;
        PerFrameGeneralPurposeDescriptorSet(DeviceContext& deviceContext);
    };



    class ColoredVertexMeshPipeline
    {
        std::shared_ptr<PerFrameGeneralPurposeDescriptorSet> _descriptorSetLayout;
        std::shared_ptr<DepthColorRenderPass>                _renderPass;
        vkr::PipelineLayout _pipelineLayout{nullptr};
        vkr::Pipeline       _pipeline{nullptr};
        void Setup(DeviceContext& deviceContext);
    public:
        ColoredVertexMeshPipeline(
            DeviceContext& deviceContext, 
            std::shared_ptr<DepthColorRenderPass> renderPass,
            std::shared_ptr<PerFrameGeneralPurposeDescriptorSet> descriptorSetLayout
        );
        ColoredVertexMeshPipeline() = default;
        ColoredVertexMeshPipeline(ColoredVertexMeshPipeline&& that) = default;
        ColoredVertexMeshPipeline& operator=(ColoredVertexMeshPipeline&& rhs) = default;

    };
}
