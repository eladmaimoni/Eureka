#pragma once
#include "DeviceContext.hpp"
#include "VkHelpers.hpp"
#include "vk_error_handling.hpp"
#include "RenderPass.hpp"
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
    //                        UIPipeline
    // 
    //////////////////////////////////////////////////////////////////////////
    class ImGuiPipeline : public PipelineBase
    {
        vk::DescriptorSetLayout _fragmentShaderSetLayout;
        void Setup(DeviceContext& deviceContext, vk::RenderPass renderPass);

        
    public:
        ImGuiPipeline(
            DeviceContext& deviceContext,
            const DepthColorRenderPass& renderPass,
            const SingleFragmentShaderCombinedImageSamplerDescriptorSetLayout& fragmentShaderSetLayout
        );
        EUREKA_DEFAULT_MOVEABLE(ImGuiPipeline);
    
        vk::DescriptorSetLayout GetFragmentShaderDescriptorSetLayout() const
        {
            return _fragmentShaderSetLayout;
        }
    };


    //////////////////////////////////////////////////////////////////////////
    //
    //                        ColoredVertexMeshPipeline
    // 
    //////////////////////////////////////////////////////////////////////////
    class ColoredVertexMeshPipeline : public PipelineBase
    {
        vk::DescriptorSetLayout _descLayout;
        void Setup(DeviceContext& deviceContext, vk::RenderPass renderPass);
    public:
        ColoredVertexMeshPipeline(
            DeviceContext& deviceContext, 
            const DepthColorRenderPass& renderPass,
            const SingleVertexShaderUBODescriptorSetLayout& descriptorSetLayout
        );
        EUREKA_DEFAULT_MOVEABLE(ColoredVertexMeshPipeline);
    
        vk::DescriptorSetLayout GetPerViewLayout() const 
        {
            return _descLayout;
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
            const SingleVertexShaderUBODescriptorSetLayout& perViewDescriptorSetLayout,
            const ColorAndNormalMapFragmentDescriptorSetLayout& perNormalMappedModelDescriptorSetLayout
        );
        EUREKA_DEFAULT_MOVEABLE(PhongShadedMeshWithNormalMapPipeline);
    };

    //////////////////////////////////////////////////////////////////////////
    //
    //                        PiplinesCache
    // 
    //////////////////////////////////////////////////////////////////////////

    template<typename P>
    struct CachedPipeline
    {
        using pipeline_type = P;
        std::mutex         mtx;
        std::shared_ptr<P> p;
    };

    class PipelineCache
    {
    private:
        DeviceContext& _deviceContext;     
        // NOTE: if we allow multiple render passes, we should use a different pipeline cache
        // but the same descriptor set layouts
        // we should probably have a pipeline cache per render pass instance
        std::shared_ptr<DepthColorRenderPass>                       _depthColorRenderPass;
        SingleVertexShaderUBODescriptorSetLayout                    _singleVertexShaderUBODSL;
        ColorAndNormalMapFragmentDescriptorSetLayout                _perNormalMappedModelDSL;
        SingleFragmentShaderCombinedImageSamplerDescriptorSetLayout _singleFragmentShaderCISDSL;

        CachedPipeline<ColoredVertexMeshPipeline>                   _coloredVertexMeshPipeline;
        CachedPipeline<PhongShadedMeshWithNormalMapPipeline>        _phongShadedMeshWithNormalMapPipeline;
        CachedPipeline<ImGuiPipeline>                               _imguiPipeline;


        template<typename CachedPipeline, typename ... Args>
        std::shared_ptr<typename CachedPipeline::pipeline_type> GetCachedPipeline(CachedPipeline& cachedPipeline, Args&& ... args)
        {
            auto ptr = cachedPipeline.p;

            if (!ptr)
            {
                std::scoped_lock lk(cachedPipeline.mtx);
                if (!cachedPipeline.p)
                {
                    cachedPipeline.p = std::make_shared<typename CachedPipeline::pipeline_type>(
                        _deviceContext,
                        std::forward<Args>(args)...
                        );
                }
                ptr = cachedPipeline.p;
            }
            return ptr;
        }


    public:
        PipelineCache(
            DeviceContext& deviceContext,
            std::shared_ptr<DepthColorRenderPass> depthColorRenderPass
        )
            : 
            _deviceContext(deviceContext),
            _depthColorRenderPass(std::move(depthColorRenderPass)),
            _singleVertexShaderUBODSL(deviceContext),
            _perNormalMappedModelDSL(deviceContext),
            _singleFragmentShaderCISDSL(deviceContext)
        {

        }


        const ColorAndNormalMapFragmentDescriptorSetLayout& GetColorAndNormalMapFragmentDescriptorSetLayout()
        {
            return _perNormalMappedModelDSL;
        }

        std::shared_ptr<ColoredVertexMeshPipeline> GetColoredVertexMeshPipeline()
        {
            return GetCachedPipeline(
                _coloredVertexMeshPipeline,
                *_depthColorRenderPass,
                _singleVertexShaderUBODSL
            );
        }
        std::shared_ptr<ImGuiPipeline> GetImGuiPipeline()
        {
            return GetCachedPipeline(
                _imguiPipeline,
                *_depthColorRenderPass,
                _singleFragmentShaderCISDSL
            );
        }
        std::shared_ptr<PhongShadedMeshWithNormalMapPipeline> GetPhongShadedMeshWithNormalMapPipeline()
        {
            return GetCachedPipeline(
                _phongShadedMeshWithNormalMapPipeline,
                *_depthColorRenderPass,
                _singleVertexShaderUBODSL,
                _perNormalMappedModelDSL
                );
        }
    };
}
