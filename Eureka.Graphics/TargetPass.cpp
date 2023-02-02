#include "TargetPass.hpp"
#include <profiling.hpp>

namespace eureka::graphics
{


    SwapChainDepthColorPass::SwapChainDepthColorPass(
        GlobalInheritedData globalInheritedData,
        vulkan::Queue graphicsQueue,
        std::shared_ptr<vulkan::SwapChain> swapChain
    ) :
        ITargetPass(std::move(globalInheritedData)),
        _graphicsQueue(graphicsQueue),
        _swapChain(std::move(swapChain))
    {
        _maxFramesInFlight = _swapChain->ImageCount();

        vulkan::DepthColorRenderPassConfig depthColorConfig
        {
            .color_output_format = _swapChain->ImageFormat(),
            .depth_output_format = vulkan::GetDefaultDepthBufferFormat(*_globalInheritedData.device)
        };

        _renderPass = std::make_shared<vulkan::DepthColorRenderPass>(_globalInheritedData.device, depthColorConfig);


        _resizeConnection = _swapChain->ConnectResizeSlot(
            [this](uint32_t w, uint32_t h)
            {
                HandleSwapChainResize(w, h);
            }
        );

        _renderTargets = vulkan::CreateDepthColorTargetForSwapChain(_globalInheritedData.device, _globalInheritedData.resource_allocator, *_swapChain, _renderPass);

        _currentRenderTarget = &_renderTargets[0];


    }

    void SwapChainDepthColorPass::AddViewPass(std::shared_ptr<IViewPass> viewPass)
    {
        auto& vp = _viewPasses.emplace_back(std::move(viewPass));

        vp->BindToTargetPass(TargetInheritedData{ _renderPass });
        vp->HandleResize(_width, _height);
    }

    void SwapChainDepthColorPass::Prepare()
    {
        for (auto& view : _viewPasses)
        {
            view->Prepare();
        }
    }

    TargetPassBeginInfo SwapChainDepthColorPass::PreRecord()
    {
        auto [valid, currentFrame, imageReadySemaphore] = _swapChain->AcquireNextAvailableImageAsync();

        if (valid)
        {
            // wait for current frame to finish execution before we reset its command pool (other frames can be in flight)
            _currentRenderTarget = &_renderTargets[currentFrame];

            return TargetPassBeginInfo
            {
                .valid = true,
                .target_available_wait_semaphore = imageReadySemaphore
            };
        }
        else
        {
            return TargetPassBeginInfo
            {
                .valid = false
            };
        }
    }

    void SwapChainDepthColorPass::PostRecord()
    {

    }

    void SwapChainDepthColorPass::PostSubmit(vulkan::BinarySemaphoreHandle waitSemaphore)
    {
        PROFILE_CATEGORIZED_SCOPE("Present", eureka::profiling::Color::Gray, eureka::profiling::PROFILING_CATEGORY_RENDERING);

        auto result = _swapChain->PresentLastAcquiredImageAsync(waitSemaphore);

        if (result == VkResult::VK_ERROR_OUT_OF_DATE_KHR)
        {
            //RecreateTargets();
        }
    }

    void SwapChainDepthColorPass::RecreateTargets()
    {
        _graphicsQueue.WaitIdle();
        _renderTargets = vulkan::CreateDepthColorTargetForSwapChain(_globalInheritedData.device, _globalInheritedData.resource_allocator, *_swapChain, _renderPass);
        _currentRenderTarget = &_renderTargets[0];
    }

    void SwapChainDepthColorPass::HandleSwapChainResize(uint32_t width, uint32_t height)
    {
        _width = width;
        _height = height;
        //DEBUGGER_TRACE("handle swap chain resize");

        RecreateTargets();

        for (auto& view : _viewPasses)
        {
            view->HandleResize(width, height);
        }

        _resizeSignal(width, height);
    }

    void SwapChainDepthColorPass::RecordDraw(const RecordParameters& params)
    {
        params.command_buffer.BeginRenderPass(_currentRenderTarget->BeginInfo());

        for (auto& view : _viewPasses)
        {
            view->RecordDraw(params);
        }

        params.command_buffer.EndRenderPass();
    }

}

