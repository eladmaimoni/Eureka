#pragma once
#include "IPass.hpp"
#include "RenderTarget.hpp"

namespace eureka
{
    class SwapChainDepthColorPass : public ITargetPass
    {
    public:
        SwapChainDepthColorPass(
            DeviceContext& deviceContext,
            Queue graphicsQueue,
            std::shared_ptr<SwapChain> swapChain
        );
        
        void AddViewPass(std::shared_ptr<IViewPass> viewPass)
        {
            _viewPasses.emplace_back(std::move(viewPass));
        }

        void Prepare() override;
        TargetPassBeginInfo PreRecord() override;
        void RecordDraw(const RecordParameters& params) override;
        void PostRecord() override;
        void PostSubmit(vk::Semaphore waitSemaphore) override;
        vk::RenderPass GetRenderPass()
        {
            return _renderPass->Get();
        }
        vk::Extent2D GetSize() override
        {
            return _swapChain->RenderArea().extent;
        }

    private:
        DeviceContext& _deviceContext;
        Queue                                    _graphicsQueue;
        std::shared_ptr<SwapChain>               _swapChain;
        std::vector<DepthColorRenderTarget>      _renderTargets;
        std::shared_ptr<DepthColorRenderPass>    _renderPass;
        uint32_t                                 _maxFramesInFlight{};
        DepthColorRenderTarget*                  _currentRenderTarget{ nullptr };
        std::vector<std::shared_ptr<IViewPass>>  _viewPasses;
    private:
        void HandleSwapChainResize(uint32_t width, uint32_t height);
    };
}

