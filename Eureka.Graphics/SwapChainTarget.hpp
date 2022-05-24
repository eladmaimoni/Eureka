

#include <VkRuntime.hpp>

namespace eureka
{
    struct SwapChainTargetDesc
    {
        vk::SurfaceKHR                            surface;
        vk::raii::PhysicalDevice*                 physical_device;
        uint32_t                                  width;
        uint32_t                                  height;

    };

    class SwapChainTarget
    {
    public:
        SwapChainTarget(const GPURuntime& runtime, SwapChainTargetDesc desc);
        vk::SurfaceKHR         _surface;
        vk::Format             _swapchainFormat;
        vk::Extent2D           _swapchainExtent;
        vk::SwapchainKHR       _swapchain;
        std::vector<vk::Image> _swapchainImages;
    };
}