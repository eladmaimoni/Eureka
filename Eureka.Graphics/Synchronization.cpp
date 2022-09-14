#include "Synchronization.hpp"

namespace eureka
{
    Semaphore::Semaphore(vk::Device device) : _device(device)
    {

    }

    Semaphore::Semaphore(Semaphore&& that) :
        _device(that._device), _semaphore(that._semaphore)
    {
        that._semaphore = nullptr;
    }

    vk::Semaphore Semaphore::Get() const
    {
        return _semaphore;
    }

    Semaphore& Semaphore::operator=(Semaphore&& rhs)
    {
        _device = rhs._device;
        _semaphore = rhs._semaphore;

        rhs._semaphore = nullptr;

        return *this;
    }

    Semaphore::~Semaphore()
    {
    }

    TimelineSemaphoreBase::TimelineSemaphoreBase(vk::Device device, uint64_t initialValue /*= 0*/) : Semaphore(device)
    {
        vk::SemaphoreTypeCreateInfo semaphoreTypeCreateInfo
        {
            .semaphoreType = vk::SemaphoreType::eTimeline,
            .initialValue = initialValue
        };

        vk::SemaphoreCreateInfo semaphoreCreateInfo
        {
            .pNext = &semaphoreTypeCreateInfo
        };

        _semaphore = _device.createSemaphore(semaphoreCreateInfo);
    }

    TimelineSemaphoreBase::~TimelineSemaphoreBase()
    {

    }

    SemaphoreStatus TimelineSemaphoreBase::WaitFor(uint64_t value, std::chrono::nanoseconds timeout)
    {
        vk::SemaphoreWaitInfo waitInfo
        {
            .semaphoreCount = 1,
            .pSemaphores = &_semaphore,
            .pValues = &value
        };

        auto result = _device.waitSemaphores(waitInfo, timeout.count());

        if (result == vk::Result::eTimeout)
        {
            return SemaphoreStatus::eTimeout;
        }
        else if (result == vk::Result::eSuccess)
        {
            return  SemaphoreStatus::eFulfilled;
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
        return _device.getSemaphoreCounterValue(_semaphore);
    }

    void TimelineSemaphoreBase::Signal(uint64_t value)
    {
        vk::SemaphoreSignalInfo signalInfo
        {
            .semaphore = _semaphore,
            .value = value
        };

        _device.signalSemaphore(signalInfo);
    }

    BinarySemaphoreBase::BinarySemaphoreBase(vk::Device device) : Semaphore(device)
    {
        vk::SemaphoreCreateInfo semaphoreCreateInfo{};
        _semaphore = device.createSemaphore(semaphoreCreateInfo);
    }

    BinarySemaphoreBase::~BinarySemaphoreBase()
    {

    }

    BinarySemaphore::~BinarySemaphore()
    {
        if (_semaphore)
        {
            _device.destroySemaphore(_semaphore);
        }
    }

    TimelineSemaphore::~TimelineSemaphore()
    {
        if (_semaphore)
        {
            _device.destroySemaphore(_semaphore);
        }
    }

    CounterSemaphore::~CounterSemaphore()
    {
        if (_semaphore)
        {
            _device.destroySemaphore(_semaphore);
        }
    }

}

