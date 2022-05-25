

#include <VkRuntime.hpp>

namespace eureka
{
    struct SwapChainTargetDesc
    {
        vk::raii::SurfaceKHR                      surface{nullptr};
        vk::raii::PhysicalDevice*                 physical_device;
        vk::raii::Device*                         logical_device;

        uint32_t                                  width;
        uint32_t                                  height;
        uint32_t                                  present_queue_family;
        uint32_t                                  graphics_queue_family;


    };

    class SwapChainTarget
    {
    public:
        SwapChainTarget(const GPURuntime& runtime, SwapChainTargetDesc desc);
        vk::raii::SurfaceKHR             _surface;
        vk::Format                       _swapchainFormat;
        vk::Extent2D                     _swapchainExtent;
        vk::raii::SwapchainKHR           _swapchain{ nullptr };
        std::vector<VkImage>             _images;
    };
}