#pragma once


/*
https://www.youtube.com/watch?v=ErtSXzVG7nU
https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Base_code
https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance
*/
namespace eureka
{
    inline constexpr char VK_LAYER_VALIDATION[] = "VK_LAYER_KHRONOS_validation";

    struct GPURuntimeDesc
    {
        std::vector<const char*> required_instance_extentions;
        std::vector<const char*> required_layers;
    };

    class GPURuntime
    {
    private:
        vk::raii::Context _context;
        vk::raii::Instance _instance;
        vk::raii::DebugUtilsMessengerEXT _debugMessenger;



    public:
        GPURuntime(const GPURuntimeDesc& desc);
        ~GPURuntime();
        const vk::raii::Instance& Instance() const { return _instance; }

    };

    struct QueueFamilies
    {
        uint32_t                               direct_graphics_family_index;
        uint32_t                               present_family_index;
        uint32_t                               copy_family_index;
        uint32_t                               compute_family_index;
    };

    struct VkDeviceContextDesc
    {
        std::vector<const char*> required_layers;
        std::vector<const char*> required_extentions;
        vk::SurfaceKHR           presentation_surface;
    };

    class VkDeviceContext
    {
        std::shared_ptr <vk::raii::PhysicalDevice>          _physicalDevice{nullptr};
        std::shared_ptr<vk::raii::Device>                   _device;
        std::shared_ptr<vk::raii::Queue>                    _graphicsQueue;
        std::shared_ptr<vk::raii::Queue>                    _computeQueue;
        std::shared_ptr<vk::raii::Queue>                    _copyQueue;
        std::shared_ptr<vk::raii::Queue>                    _presentQueue;
        QueueFamilies                                       _families;
    public:
        VkDeviceContext(const vk::raii::Instance& instance, const VkDeviceContextDesc& desc);
        ~VkDeviceContext();


        // TODO: is there a point in shared_ptr for a queue? it is just a handle that is really attached to the device
        // no sense in being move-only?
        const std::shared_ptr<vk::raii::Device>& Device() const { return _device;};
        const std::shared_ptr<vk::raii::Queue>& GraphicsQueue() const { return _graphicsQueue; };
        const std::shared_ptr<vk::raii::Queue>& ComputeQueue() const { return _computeQueue; };
        const std::shared_ptr<vk::raii::Queue>& CopyQueue() const { return _copyQueue; };
        const std::shared_ptr<vk::raii::Queue>& PresentQueue() const { return _presentQueue; };
        const QueueFamilies& Families() const { return _families; }
        const std::shared_ptr<vk::raii::PhysicalDevice> PhysicalDevice() const { return _physicalDevice; }
    private:
        void InitDeviceAndQueues(const vk::raii::Instance& instance, const VkDeviceContextDesc& desc);
    };

}

/*
Note about Dispatcher.

by default, each call to a method in Vulkan-Hpp takes a Dispatch template argument.

somehow, without providing the template type or the Dispatch argument, the API
passes a default DispatchLoaderBase implementation.
this implementation is the DispatchLoaderStatic that has methods to call all (?) vulkan c api 
which are statically linked.

When using the raii::Context together with other raii objects, we no longer have the dispatch parameter.
instead, the context itself instantiates a DispatchLoaderBase parameter that loads the API dynamically.

This means we do not need to link against Vulkan static library?


this Dispatch object is statically created and calls vulkan methods that are statically
linked to the vulkan-1.lib or something.

if you use
*/