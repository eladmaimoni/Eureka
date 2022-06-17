#pragma once
#include "IPass.hpp"
#include "RenderTarget.hpp"
#include "Camera.hpp"
#include "Pipeline.hpp"
#include "Descriptors.hpp"

namespace eureka
{
    class ColoredMesh
    {
        VertexAndIndexTransferableDeviceBuffer _triangle; // TODO remove
    };

    class ColoredMeshPass : public IMeshPass
    {
        
        
        std::shared_ptr<ColoredVertexMeshPipeline> _coloredVertexPipeline;
    
        void Prepare() override
        {

        }
        void RecordDraw(const RecordParameters& /*params*/) override
        {
            //params.command_buffer.bindDescriptorSets(
            //    vk::PipelineBindPoint::eGraphics,
            //    _coloredVertexPipeline->Layout(),
            //    0,
            //    { _constantBufferSet.Get() },
            //    nullptr
            //);
        }
    };

    class CameraPass : public IViewPass
    {
        std::shared_ptr<MTDescriptorAllocator> _setAllocator; // TODO move to device
        std::vector<std::shared_ptr<IMeshPass>> _meshPasses;
        std::shared_ptr<CameraNode> _cameraNode;
        FreeableDescriptorSet       _set0PerView;
    public:
        CameraPass(
            std::shared_ptr<CameraNode> cameraNode,
            std::shared_ptr<DescriptorSetLayoutCache> setLayoutCache, // TODO unify
            std::shared_ptr<MTDescriptorAllocator> setAllocator // TODO unify
        )
            : 
            _cameraNode(std::move(cameraNode)),
            _setAllocator(setAllocator)
        {
            auto layout = setLayoutCache->RetrieveLayout(0, SET0_ID_000_PER_VIEW);
            _set0PerView = setAllocator->AllocateSet(layout);

            auto [descType, descInfo] = _cameraNode->DescriptorInfo();

            _set0PerView.SetBinding(0, descType, descInfo);
        }

        void Prepare() override
        {
            for (auto& meshPass : _meshPasses)
            {
                meshPass->Prepare();
            }
        }

        void RecordDraw(const RecordParameters& params) override
        {
            auto vp = _cameraNode->Viewport();
            vk::Rect2D fullRect{ {0,0}, {static_cast<uint32_t>(vp.width), static_cast<uint32_t>(vp.height)} };
            params.command_buffer.setViewport(0, vp);         
            params.command_buffer.setScissor(0, fullRect);
            
            if (!_meshPasses.empty())
            {
                params.command_buffer.bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics,
                    _meshPasses[0]->GetPipelineLayout(),
                    0,
                    _set0PerView.Get(),
                    nullptr
                );


                _cameraNode->SyncTransforms(); // can be potentially be after recording 

                for (auto& meshPass : _meshPasses)
                {
                    meshPass->RecordDraw(params);
                }
            }

        }

        void HandleResize(uint32_t w, uint32_t h)
        {
            _cameraNode->SetViewport(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h));
        }

    };




}

