#pragma once
#include "IPass.hpp"
#include "../Eureka.Vulkan/RenderTarget.hpp"
#include "../Eureka.Vulkan/RenderPass.hpp"

namespace eureka::graphics
{


    class SwapChainDepthColorPass : public ITargetPass
    {
    public:
        SwapChainDepthColorPass(
            GlobalInheritedData globalInheritedData,
            vulkan::Queue graphicsQueue,
            std::shared_ptr<vulkan::SwapChain> swapChain
        );
        
        void AddViewPass(std::shared_ptr<IViewPass> viewPass);

        void Prepare() override;
        TargetPassBeginInfo PreRecord() override;
        void RecordDraw(const RecordParameters& params) override;
        void PostRecord() override;
        void PostSubmit(vulkan::BinarySemaphoreHandle waitSemaphore) override;
        

        VkRenderPass GetRenderPass()
        {
            return _targetInheritedData.render_pass->Get();
        }

        VkExtent2D GetSize() override
        {
            return _swapChain->RenderArea().extent;
        }

    private:
        //std::shared_ptr<vulkan::Device>                  _device;
        //std::shared_ptr<vulkan::ResourceAllocator>       _allocator;
        vulkan::Queue                                    _graphicsQueue;
        std::shared_ptr<vulkan::SwapChain>               _swapChain;
        std::vector<vulkan::DepthColorRenderTarget>      _renderTargets;
        std::shared_ptr<vulkan::DepthColorRenderPass>    _renderPass;
        uint32_t                                         _maxFramesInFlight{};
        vulkan::DepthColorRenderTarget*                  _currentRenderTarget{ nullptr };
        uint32_t                                         _width{ 0 };
        uint32_t                                         _height{ 0 };

        std::vector<std::shared_ptr<IViewPass>>          _viewPasses;
    private:
        void RecreateTargets();
        void HandleSwapChainResize(uint32_t width, uint32_t height);
    };
}

