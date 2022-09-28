#pragma once
#include <volk.h>
#include <vector>
#include <memory>
#include <optional>
#include <span>

namespace eureka
{
    template<typename T> using dspan = std::span<T, std::dynamic_extent>;
    template<typename T> using dcspan = std::span<const T, std::dynamic_extent>;
}

namespace eureka::vulkan
{
    inline constexpr char INSTANCE_LAYER_VALIDATION[] = "VK_LAYER_KHRONOS_validation";
    inline constexpr char INSTANCE_LAYER_PRE13_SYNCHRONIZATION2[] = "VK_LAYER_KHRONOS_synchronization2";

    inline constexpr char INSTANCE_EXTENTION_DEBUG_UTILS[] = "VK_EXT_debug_utils";
    inline constexpr char INSTANCE_EXTENTION_WIN32_SURFACE_EXTENSION_NAME[] = "VK_KHR_win32_surface";
    inline constexpr char INSTANCE_EXTENTION_SURFACE_EXTENSION_NAME[] = VK_KHR_SURFACE_EXTENSION_NAME;
    

    struct Version
    {
        uint32_t major;
        uint32_t minor;
        uint32_t patch;

        uint32_t Get() const
        {
            return VK_MAKE_VERSION(major, minor, patch);
        }
    };

    struct InstanceConfig
    {
        std::optional<Version> version;
        std::vector<const char*> required_instance_extentions;
        std::vector<const char*> required_layers;
    };



    class Instance
    {
    private:
        InstanceConfig _config{};
        uint32_t _apiVersion{};
        VkInstance _instance{};
        VkDebugUtilsMessengerEXT _debugMessenger{};
    public:
        Instance(const InstanceConfig& desc);
        ~Instance();
        std::vector<VkPhysicalDevice> EnumeratePhysicalDevices();
        Version ApiVersion() const;
        uint32_t RawApiVersion() const;
        VkInstance Get() const { return _instance; }
        void DestroySurface(VkSurfaceKHR surface) const;
        Instance& operator=(const Instance&) = delete;
        Instance(const Instance&) = delete;
        dspan<const char*> EnabledExtentions() const;
    };

    std::shared_ptr<Instance> MakeDefaultInstance();

}

