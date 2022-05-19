
#include <vulkan/vulkan.hpp>
#include <VkRuntime.hpp>

namespace eureka
{
    struct SwapChainTargetDesc
    {
        vk::SurfaceKHR surface;
        uint32_t width;
        uint32_t height;

    };

    class SwapChainTarget
    {
    public:
        SwapChainTarget(const VkRuntime& runtime, SwapChainTargetDesc desc)
            : _surface(std::move(desc.surface))
        {

        }
        vk::SurfaceKHR         _surface;
        vk::Format             _swapchainFormat;
        vk::Extent2D           _swapchainExtent;
        vk::SwapchainKHR       _swapchain;
        std::vector<vk::Image> _swapchainImages;
    };
}