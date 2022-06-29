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
        vk::DescriptorSetLayout _perViewLayout;
        void Setup(DeviceContext& deviceContext, vk::RenderPass renderPass);
    public:
        ColoredVertexMeshPipeline(
            DeviceContext& deviceContext, 
            const DepthColorRenderPass& renderPass,
            const PerViewDescriptorSetLayout& descriptorSetLayout
        );
        EUREKA_DEFAULT_MOVEABLE(ColoredVertexMeshPipeline);
    
        vk::DescriptorSetLayout GetPerViewLayout() const 
        {
            return _perViewLayout;
        }
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //                        PhongShadedMeshWithNormalMapPipeline
    // 
    //////////////////////////////////////////////////////////////////////////
    class PhongShadedMeshWithNormalMapPipeline : public PipelineBase
    {
        void Setup(DeviceContext& deviceContext, vk::RenderPass renderPass);
    public:
        PhongShadedMeshWithNormalMapPipeline(
            DeviceContext& deviceContext,
            const DepthColorRenderPass& renderPass,
            const PerViewDescriptorSetLayout& perViewDescriptorSetLayout,
            const PerNormalMappedModelDescriptorSetLayout& perNormalMappedModelDescriptorSetLayout
        );
        EUREKA_DEFAULT_MOVEABLE(PhongShadedMeshWithNormalMapPipeline);
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //                        PiplinesCache
    // 
    //////////////////////////////////////////////////////////////////////////
    class PiplineCache
    {
    private:
        DeviceContext& _deviceContext;
     
        std::mutex _mtx;
        // NOTE: if we allow multiple render passes, we should use a different pipeline cache
        // but the same descriptor set layouts
        // we should probably have a pipeline cache per render pass instance
        std::shared_ptr<DepthColorRenderPass>                   _depthColorRenderPass;
        PerViewDescriptorSetLayout                              _perViewDSL;
        PerNormalMappedModelDescriptorSetLayout                 _perNormalMappedModelDSL;
        std::shared_ptr<ColoredVertexMeshPipeline>              _coloredVertexMeshPipeline;
        std::shared_ptr<PhongShadedMeshWithNormalMapPipeline>   _phongShadedMeshWithNormalMapPipeline;
    public:
        PiplineCache(
            DeviceContext& deviceContext,
            std::shared_ptr<DepthColorRenderPass> depthColorRenderPass
        )
            : 
            _deviceContext(deviceContext),
            _depthColorRenderPass(std::move(depthColorRenderPass)),
            _perViewDSL(deviceContext),
            _perNormalMappedModelDSL(deviceContext)
        {

        }
        
        std::shared_ptr<ColoredVertexMeshPipeline> GetColoredVertexMeshPipeline()
        {
            auto ptr = _coloredVertexMeshPipeline;

            if (!ptr)
            {
                std::scoped_lock lk(_mtx);
                if (!_coloredVertexMeshPipeline)
                {
                    _coloredVertexMeshPipeline = std::make_shared<ColoredVertexMeshPipeline>(
                        _deviceContext, 
                        *_depthColorRenderPass,
                        _perViewDSL
                        );
                }
            
                ptr = _coloredVertexMeshPipeline;
            }

            return ptr;
        }

        std::shared_ptr<PhongShadedMeshWithNormalMapPipeline> GetPhongShadedMeshWithNormalMapPipeline()
        {
            auto ptr = _phongShadedMeshWithNormalMapPipeline;

            if (!ptr)
            {
                std::scoped_lock lk(_mtx);
                if (!_phongShadedMeshWithNormalMapPipeline)
                {
                    _phongShadedMeshWithNormalMapPipeline = std::make_shared<PhongShadedMeshWithNormalMapPipeline>(
                        _deviceContext,
                        *_depthColorRenderPass,
                        _perViewDSL,
                        _perNormalMappedModelDSL
                        );     
                }
          
                ptr = _phongShadedMeshWithNormalMapPipeline;
            }

            return ptr;
        }
    };
}
