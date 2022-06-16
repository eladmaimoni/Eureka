#pragma once
#include "Instance.hpp"
#include "vk_mem_alloc.h"
#include <ShadersCache.hpp>
#include "UpdateQueue.hpp"

namespace eureka
{
    class Queue
    {
    public:
        Queue(vk::Queue queue, uint32_t family)
            : _queue(queue), _family(family)
        {

        }

        vk::Queue* operator->()
        {
            return &_queue;
        }

        Queue() = default;

        vk::Queue Get() const
        {
            return _queue;
        }
        uint32_t Family() const
        {
            return _family;
        }
    private:
        uint32_t  _family;
        vk::Queue _queue;


    };

    struct QueueManagement
    {
        Queue queue;
        bool is_taken = false;
    };


    struct DeviceContextConfig
    {
        std::vector<const char*> required_layers;
        std::vector<const char*> required_extentions;
        vk::SurfaceKHR           presentation_surface; // optional
        uint32_t                 preferred_number_of_graphics_queues{ 1 }; 
        uint32_t                 preferred_number_of_compute_queues{ 2 }; // save one for present, TODO figure out how this should be handled if we don't know in advance which queue support presentation
        uint32_t                 preferred_number_of_copy_queues{ 1 }; // read & write 
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

        //
        // Accessors
        //

        Queue CreateGraphicsQueue();
        Queue CreateComputeQueue();
        Queue CreateCopyQueue();
        Queue CreatePresentQueue(vk::SurfaceKHR presentationSurface);

        const std::shared_ptr<vkr::PhysicalDevice> PhysicalDevice() const { assert(_device); return _physicalDevice; }
        const std::shared_ptr<vkr::Device> LogicalDevice() const { assert(_device); return _device; }
        VmaAllocator Allocator() const { return _vmaAllocator; }
        const ShaderCache& Shaders() { return _shaderCache; }
        const std::shared_ptr<RenderingUpdateQueue>& UpdateQueue() const { return _updateQueue; }
    private:
        void InitDeviceAndQueues(const vkr::Instance& instance, const DeviceContextConfig& desc);
        std::optional<Queue> TryCreateQueue(uint32_t family);
    private:
        std::shared_ptr<vkr::PhysicalDevice>           _physicalDevice{ nullptr };
        std::shared_ptr<vkr::Device>                   _device;
        std::vector<vk::Queue>                         _graphicsQueue;
        std::vector<vk::Queue>                         _computeQueue;
        std::vector<vk::Queue>                         _copyQueue;
        vk::Queue                                      _presentQueue;


        uint32_t                               _preferredGraphicsIdx;
        uint32_t                               _preferredComputeIdx;
        uint32_t                               _preferredCopyIdx;
        std::unordered_map<uint32_t, std::vector<QueueManagement>>        _queuesByFamily;



        VmaAllocator                                   _vmaAllocator{nullptr};
        ShaderCache                                    _shaderCache;
        std::shared_ptr<RenderingUpdateQueue>    _updateQueue = std::make_shared<RenderingUpdateQueue>();
    };
}