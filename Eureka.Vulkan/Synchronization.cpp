#include "Synchronization.hpp"
#include "move.hpp"

namespace eureka::vulkan
{
    Semaphore::Semaphore(std::shared_ptr<Device> device) : _device(std::move(device))
    {

    }

    Semaphore::Semaphore(Semaphore&& that) :
        _device(std::move(that._device)), _semaphore(that._semaphore)
    {
        that._semaphore = nullptr;
    }

    VkSemaphore Semaphore::Get() const
    {
        return _semaphore;
    }

    Semaphore& Semaphore::operator=(Semaphore&& rhs)
    {
        _device = std::move(rhs._device);
        _semaphore = rhs._semaphore;

        rhs._semaphore = nullptr;

        return *this;
    }

    Semaphore::~Semaphore()
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //
    //                     TimelineSemaphoreBase
    //
    //////////////////////////////////////////////////////////////////////////

    TimelineSemaphoreBase::TimelineSemaphoreBase(std::shared_ptr<Device> device, uint64_t initialValue /*= 0*/) : Semaphore(std::move(device))
    {
        VkSemaphoreTypeCreateInfo semaphoreTypeCreateInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .semaphoreType = VkSemaphoreType::VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue = initialValue
        };

        VkSemaphoreCreateInfo semaphoreCreateInfo
        { 
            .sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &semaphoreTypeCreateInfo
        };

        _semaphore = _device->CreateSemaphore(semaphoreCreateInfo);
    }

    TimelineSemaphoreBase::~TimelineSemaphoreBase()
    {

    }

    SemaphoreStatus TimelineSemaphoreBase::WaitFor(uint64_t value, std::chrono::nanoseconds timeout)
    {
        VkSemaphoreWaitInfo waitInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
            .semaphoreCount = 1,
            .pSemaphores = &_semaphore,
            .pValues = &value
        };

        auto result = _device->WaitSemaphores(waitInfo, timeout.count());

        if (result == VkResult::VK_TIMEOUT)
        {
            return SemaphoreStatus::eTimeout;
        }
        else if (result == VkResult::VK_SUCCESS)
        {
            return SemaphoreStatus::eFulfilled;
        }
        else
        {
            throw std::logic_error("bad semaphore");
        }
    }


    void TimelineSemaphoreBase::Wait(uint64_t value)
    {
        WaitFor(value, std::chrono::nanoseconds(UINT64_MAX));
    }

    uint64_t TimelineSemaphoreBase::QueryValue()
    {
        return _device->GetSemaphoreCounterValue(_semaphore);
    }

    void TimelineSemaphoreBase::Signal(uint64_t value)
    {
        VkSemaphoreSignalInfo signalInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
            .semaphore = _semaphore,
            .value = value
        };

        _device->SignalSemaphore(signalInfo);
    }

    //////////////////////////////////////////////////////////////////////////
    //
    //                     BinarySemaphoreBase
    //
    //////////////////////////////////////////////////////////////////////////

    BinarySemaphoreBase::BinarySemaphoreBase(std::shared_ptr<Device> device) : Semaphore(std::move(device))
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };
        _semaphore = _device->CreateSemaphore(semaphoreCreateInfo);
    }

    BinarySemaphoreBase::~BinarySemaphoreBase()
    {

    }

    BinarySemaphore::~BinarySemaphore()
    {
        if (_semaphore)
        {
            _device->DestroySemaphore(_semaphore);
        }
    }

    TimelineSemaphore::~TimelineSemaphore()
    {
        if (_semaphore)
        {
            _device->DestroySemaphore(_semaphore);
        }
    }

    CounterSemaphore::~CounterSemaphore()
    {
        if (_semaphore)
        {
            _device->DestroySemaphore(_semaphore);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    Fence::Fence(std::shared_ptr<Device> device, bool signaled /*= false*/) : _device(std::move(device))
    {
        VkFenceCreateInfo fenceCreateInfo
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = signaled ? VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT : 0u
        };

        _fence = _device->CreateFence(fenceCreateInfo);
    }

    Fence::Fence(Fence&& that) :
        _device(std::move(that._device)),
        _fence(steal(that._fence))
    {

    }
    
    Fence& Fence::operator=(Fence&& rhs)
    {
        _device = std::move(rhs._device);
        _fence = steal(rhs._fence);
        return *this;
    }

    Fence::~Fence()
    {
        if (_fence)
        {
            _device->DestroyFence(_fence);
        }
    }

    void Fence::WaitAndReset() const
    {
        _device->WaitForFence(_fence);
        _device->ResetFence(_fence);
    }


}

