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

        EUREKA_DEFAULT_MOVEABLE(PipelineBase);
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
        std::shared_ptr<PerViewDescriptorSetLayout> _perViewDescriptorSetLayout;
        void Setup(DeviceContext& deviceContext);
    public:
        ColoredVertexMeshPipeline(
            DeviceContext& deviceContext, 
            std::shared_ptr<DepthColorRenderPass> renderPass,
            std::shared_ptr<PerViewDescriptorSetLayout> descriptorSetLayout
        );
        EUREKA_DEFAULT_MOVEABLE(ColoredVertexMeshPipeline);
    };

    class PhongShadedMeshWithNormalMapPipeline : public PipelineBase
    {
        std::shared_ptr<DepthColorRenderPass>                    _renderPass;
        std::shared_ptr<PerViewDescriptorSetLayout>              _perViewDescriptorSetLayout;
        std::shared_ptr<PerNormalMappedModelDescriptorSetLayout> _perNormalMappedModelDescriptorSetLayout;
        void Setup(DeviceContext& deviceContext);
    public:
        PhongShadedMeshWithNormalMapPipeline(
            DeviceContext& deviceContext,
            std::shared_ptr<DepthColorRenderPass> renderPass,
            std::shared_ptr<PerViewDescriptorSetLayout> perViewDescriptorSetLayout,
            std::shared_ptr<PerNormalMappedModelDescriptorSetLayout> perNormalMappedModelDescriptorSetLayout
        );
        EUREKA_DEFAULT_MOVEABLE(PhongShadedMeshWithNormalMapPipeline);
    };
}
