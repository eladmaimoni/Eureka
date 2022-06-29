#pragma once
#include "DeviceContext.hpp"
#include "VkHelpers.hpp"
#include "vk_error_handling.hpp"
#include "RenderPass.hpp"
#include "Mesh.hpp"
#include "Descriptors.hpp"

namespace eureka
{  



    //////////////////////////////////////////////////////////////////////////
    //
    //                        PipelineBase
    // 
    //////////////////////////////////////////////////////////////////////////
    class PipelineBase
    {
    protected:
        vkr::PipelineLayout _pipelineLayout{ nullptr };
        vkr::Pipeline       _pipeline{ nullptr };

        ~PipelineBase() = default;
        PipelineBase() = default;
        PipelineBase(PipelineBase&& that) = default;
        PipelineBase& operator=(PipelineBase&& rhs) = default;
    public:
        vk::PipelineLayout Layout() const
        {
            return *_pipelineLayout;
        }

        vk::Pipeline Get() const
        {
            return *_pipeline;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //                        ColoredVertexMeshPipeline
    // 
    //////////////////////////////////////////////////////////////////////////
    class ColoredVertexMeshPipeline : public PipelineBase
    {
        std::shared_ptr<DepthColorRenderPass>                _renderPass;
        std::shared_ptr<PerFrameGeneralPurposeDescriptorSetLayout> _descriptorSetLayout;
        void Setup(DeviceContext& deviceContext);
    public:
        ColoredVertexMeshPipeline(
            DeviceContext& deviceContext, 
            std::shared_ptr<DepthColorRenderPass> renderPass,
            std::shared_ptr<PerFrameGeneralPurposeDescriptorSetLayout> descriptorSetLayout
        );
        ~ColoredVertexMeshPipeline() = default;
        ColoredVertexMeshPipeline() = default;
        ColoredVertexMeshPipeline(ColoredVertexMeshPipeline&& that) = default;
        ColoredVertexMeshPipeline& operator=(ColoredVertexMeshPipeline&& rhs) = default;
    };

    class PhongShadedMeshWithNormalMapPipeline : public PipelineBase
    {
        std::shared_ptr<DepthColorRenderPass>                _renderPass;
        std::shared_ptr<PerFrameGeneralPurposeDescriptorSetLayout> _descriptorSetLayout;
        void Setup(DeviceContext& deviceContext);
    public:
        PhongShadedMeshWithNormalMapPipeline(
            DeviceContext& deviceContext,
            std::shared_ptr<DepthColorRenderPass> renderPass,
            std::shared_ptr<PerFrameGeneralPurposeDescriptorSetLayout> descriptorSetLayout
        );
        ~PhongShadedMeshWithNormalMapPipeline() = default;
        PhongShadedMeshWithNormalMapPipeline() = default;
        PhongShadedMeshWithNormalMapPipeline(PhongShadedMeshWithNormalMapPipeline&& that) = default;
        PhongShadedMeshWithNormalMapPipeline& operator=(PhongShadedMeshWithNormalMapPipeline&& rhs) = default;
    };
}
