#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <volk.h>
#include "Instance.hpp"
#include "Queue.hpp"
#include <span>

namespace eureka
{
    template<typename T>
    using dynamic_cspan = std::span<const T, std::dynamic_extent>;
}

namespace eureka::vulkan
{
    inline constexpr uint32_t INVALID_IDX = std::numeric_limits<uint32_t>::max();

    inline constexpr char DEVICE_LAYER_VALIDATION[] = "VK_LAYER_KHRONOS_validation";
    inline constexpr char DEVICE_LAYER_PRE13_SYNCHRONIZATION2[] = "VK_LAYER_KHRONOS_synchronization2";
    inline constexpr char DEVICE_EXTENTION_PRE13_SYNCHRONIZATION2[] = "VK_KHR_synchronization2";
    inline constexpr char DEVICE_EXTENTION_SWAPCHAIN[] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    
    struct DeviceConfig
    {
        std::vector<const char*> required_layers;
        std::vector<const char*> required_extentions;
        VkSurfaceKHR             presentation_surface; // optional
        uint32_t                 preferred_number_of_graphics_queues{ 1 };
        uint32_t                 preferred_number_of_compute_queues{ 1 }; // save one for present, TODO figure out how this should be handled if we don't know in advance which queue support presentation
        uint32_t                 preferred_number_of_copy_queues{ 1 }; // read & write 
    };

    struct DeviceCreationDesc
    {
        std::vector<float>                     queue_priorities;
        std::vector<VkDeviceQueueCreateInfo>   queu_create_info;
        std::unordered_map<uint32_t, uint32_t> queues_per_index;

        uint32_t                               graphics_family;
        uint32_t                               compute_family;
        uint32_t                               copy_family;
        uint32_t                               presentation_family = INVALID_IDX;

        uint32_t                               graphics_index;
        uint32_t                               compute_index;
        uint32_t                               copy_index;
        uint32_t                               presentation_index = INVALID_IDX;
    };

    enum class QueueType
    {
        Graphics,
        Compute,
        Copy
    };

    struct SwapChainSupport
    {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   present_modes;
    };
    struct SwapChainImageAquisition
    {
        VkResult result{};
        uint32_t index{};
    };

    class Device
    {
        DeviceConfig                           _config;
        std::string                            _prettyName;
        std::shared_ptr<Instance>              _instance{};

        VkPhysicalDevice                       _physicalDevice{};
        VkDevice                               _logicalDevice{};

        uint32_t                               _preferredGraphicsFamily{};
        uint32_t                               _preferredComputeFamily{};
        uint32_t                               _preferredCopyFamily{};
        uint32_t                               _preferredPresentationFamily = INVALID_IDX;

        uint32_t                               _preferredGraphicsIndex{};
        uint32_t                               _preferredComputeIndex{};
        uint32_t                               _preferredCopyIndex{};
        uint32_t                               _preferredPresentationIndex = INVALID_IDX;

        std::unordered_map<uint32_t, uint32_t> _queuesPerFamily;

      
        std::tuple<VkPhysicalDevice, std::string> ChoosePhysicalDevice(const DeviceConfig& config);
        DeviceCreationDesc MakeDeviceCreationDesc(const DeviceConfig& config);
        bool FamiliySupportsPresentation(uint32_t family, VkSurfaceKHR presentationSurface);
    public:
        Device(std::shared_ptr<Instance> instance, DeviceConfig config);
        ~Device();
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;
        dspan<const char*> EnabledExtentions() const;
        std::string_view GetPrettyName() const;
        VkDevice GetDevice() const;
        VkPhysicalDevice GetPhysicalDevice() const;
        Queue GetGraphicsQueue();
        Queue GetComputeQueue();
        Queue GetCopyQueue();
        Queue GetPresentQueue();
        VkSampler CreateSampler(const VkSamplerCreateInfo& samplerCreateInfo) const;
        void DestroySampler(VkSampler sampler) const;
        VkImageView CreateImageView(const VkImageViewCreateInfo& imageViewCreateInfo) const;
        void DestroyImageView(VkImageView view) const;
        VkSemaphore CreateSemaphore(VkSemaphoreCreateInfo semaphoreCreateInfo) const;
        void DestroySemaphore(VkSemaphore semaphore) const;
        uint64_t GetSemaphoreCounterValue(VkSemaphore semaphore) const;
        void SignalSemaphore(const VkSemaphoreSignalInfo& signalInfo) const;
        VkResult WaitSemaphores(const VkSemaphoreWaitInfo& waitInfo, uint64_t timeout) const;
        VkSurfaceCapabilitiesKHR GetSwapchainCapabilities(VkSurfaceKHR surface) const;
        SwapChainSupport GetSwapChainSupport(VkSurfaceKHR surface) const;
        VkSwapchainKHR CreateSwapchain(const VkSwapchainCreateInfoKHR& createInfo) const;
        std::vector<VkImage> GetSwapchainImages(VkSwapchainKHR swapchain) const;
        SwapChainImageAquisition AcquireNextSwapChainImage(VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore imageReady, VkFence fence) const;
        void DestroySwapChain(VkSwapchainKHR _swapchain) const;
        VkPipelineCache CreatePipelineCache() const;
        void DestroyPipelineCache(VkPipelineCache pipelineCache) const;
        VkShaderModule CreateShaderModule(const VkShaderModuleCreateInfo& shaderModuleCreateInfo) const;
        void DestroyShaderModule(VkShaderModule shaderModule) const;
        VkCommandPool CreateCommandPool(const VkCommandPoolCreateInfo& commandPoolCreateInfo) const;
        void ResetCommandPool(VkCommandPool commandPool) const;
        void DestroyCommandPool(VkCommandPool commandPool) const;
        VkCommandBuffer AllocatePrimaryCommandBuffer(VkCommandPool commandPool) const;
        void DestroyRenderPass(VkRenderPass renderPass) const;
        VkRenderPass CreateRenderPass(const VkRenderPassCreateInfo& renderPassCreateInfo) const;
        VkFramebuffer CreateFrameBuffer(const VkFramebufferCreateInfo& framebufferCreateInfo) const;
        void DestroyFrameBuffer(VkFramebuffer frameBuffer) const;
        VkFormatProperties GetFormatProperties(VkFormat format) const;
        void UpdateDescriptorSet(const VkWriteDescriptorSet& writeDescriptorSet) const;
        void UpdateDescriptorSet(dynamic_cspan<VkWriteDescriptorSet> writeDescriptorSet) const;
        VkDescriptorPool CreateDescriptorPool(const VkDescriptorPoolCreateInfo& descriptorPoolCreateInfo) const;
        void DestroyDescriptorPool(VkDescriptorPool pool) const;
        void FreeDescriptorSet(VkDescriptorPool pool, VkDescriptorSet set) const;
        VkDescriptorSet AllocateDescriptorSet(const VkDescriptorSetAllocateInfo& allocInfo) const;
        VkDescriptorSet TryAllocateDescriptorSet(const VkDescriptorSetAllocateInfo& allocInfo) const;
        VkFence CreateFence(const VkFenceCreateInfo& fenceCreateInfo) const;
        void DestroyFence(VkFence fence) const;
        void WaitForFence(VkFence fence) const;
        void WaitForFences(dynamic_cspan<VkFence> fences) const;
        void ResetFence(VkFence fence) const;
        void ResetFences(dynamic_cspan<VkFence> fences) const;
        VkDescriptorSetLayout CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& info) const;
        void DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) const;
        VkPipelineLayout CreatePipelineLayout(const VkPipelineLayoutCreateInfo& pipelineLayoutCreateInfo) const;
        void DestroyPipelineLayout(VkPipelineLayout pipelineLayout) const;
        VkPipeline CreatePipeline(const VkGraphicsPipelineCreateInfo& pipelineCreateInfo, VkPipelineCache pipelineCache = nullptr) const;
        void DestroyPipeline(VkPipeline pipeline) const;
    };

    std::shared_ptr<Device> MakeDefaultDevice(std::shared_ptr<Instance> instance, VkSurfaceKHR presentationSurface = nullptr);
}

