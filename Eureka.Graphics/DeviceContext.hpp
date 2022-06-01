#pragma once
#include "Instance.hpp"
#include "vk_mem_alloc.h"

namespace eureka
{
    struct QueueFamilies
    {
        uint32_t                               direct_graphics_family_index;
        uint32_t                               direct_graphics_family_max_count;
        uint32_t                               copy_family_index;
        uint32_t                               copy_family_max_count;
        uint32_t                               compute_family_index;
        uint32_t                               compute_family_max_count;

        uint32_t                               present_family_index; // probably refers to one of the existing families
    };

    struct DeviceContextConfig
    {
        std::vector<const char*> required_layers;
        std::vector<const char*> required_extentions;
        vk::SurfaceKHR           presentation_surface; // optional
        uint32_t                 preferred_number_of_graphics_queues{ 1 }; 
        uint32_t                 preferred_number_of_compute_queues{ 2 }; // save one for present, TODO figure out how this should be handled if we don't know in advance which queue support presentation
        uint32_t                 preferred_number_of_copy_queues{ 2 }; // read & write 
    };

    class DeviceContext
    {
    public:
        DeviceContext() = default;
        DeviceContext(const Instance& instance, const DeviceContextConfig& desc);
        ~DeviceContext();
        
        //
        // Initialization
        //
        void Init(const Instance& instance, const DeviceContextConfig& desc);
        void InitializePresentationQueueFromExistingQueues(vk::SurfaceKHR presentationSurface);

        //
        // Accessors
        //
        const std::shared_ptr<vkr::Device>& Device() const { assert(_device); return _device; };
        const std::vector<std::shared_ptr<vkr::Queue>>& GraphicsQueue() const { assert(_device); return _graphicsQueue; };
        const std::vector<std::shared_ptr<vkr::Queue>>& ComputeQueue() const { assert(_device); return _computeQueue; };
        const std::vector<std::shared_ptr<vkr::Queue>>& CopyQueue() const { assert(_device); return _copyQueue; };
        const std::shared_ptr<vkr::Queue>& PresentQueue() const;
        const QueueFamilies& Families() const { assert(_device); return _families; }
        const std::shared_ptr<vkr::PhysicalDevice> PhysicalDevice() const { assert(_device); return _physicalDevice; }
        const std::shared_ptr<vkr::Device> LogicalDevice() const { assert(_device); return _device; }
        VmaAllocator Allocator() const { return _vmaAllocator; }
    private:
        void InitDeviceAndQueues(const vkr::Instance& instance, const DeviceContextConfig& desc);

    private:
        std::shared_ptr<vkr::PhysicalDevice>           _physicalDevice{ nullptr };
        std::shared_ptr<vkr::Device>                   _device;
        std::vector<std::shared_ptr<vkr::Queue>>       _graphicsQueue;
        std::vector<std::shared_ptr<vkr::Queue>>       _computeQueue;
        std::vector<std::shared_ptr<vkr::Queue>>       _copyQueue;
        std::shared_ptr<vkr::Queue>                    _presentQueue;
        QueueFamilies                                  _families;

        VmaAllocator                                   _vmaAllocator;
    };
}