#pragma once
#include "DeviceContext.hpp"
#include <macros.hpp>

namespace eureka
{
    class Semaphore
    {
    protected:
        vk::Semaphore _semaphore{ nullptr };
        vk::Device _device{ nullptr };
        Semaphore() = default;
        Semaphore(vk::Device device);
        ~Semaphore();
        Semaphore(const Semaphore& semaphore) = default;
        Semaphore& operator=(const Semaphore& semaphore) = default;
        Semaphore(Semaphore&& that);
        Semaphore& operator=(Semaphore&& rhs);
    public:
        vk::Semaphore Get() const;
    };

    enum class SemaphoreStatus
    {
        eFulfilled = 0,
        eTimeout = 2
    };

    class TimelineSemaphoreBase : public Semaphore
    {
    protected:
        TimelineSemaphoreBase(vk::Device device, uint64_t initialValue = 0);
        ~TimelineSemaphoreBase();
        EUREKA_DEFAULT_MOVEABLE_COPYABLE(TimelineSemaphoreBase);
    public:
        SemaphoreStatus WaitFor(uint64_t value, std::chrono::nanoseconds timeout);
        void Wait(uint64_t value);
        uint64_t QueryValue();
        void Signal(uint64_t value);
    };

    class TimelineSemaphore : public TimelineSemaphoreBase
    {
    public:
        EUREKA_DEFAULT_MOVEONLY(TimelineSemaphore);
        TimelineSemaphore(vk::Device device, uint64_t initialValue = 0) : TimelineSemaphoreBase(device, initialValue) {}
        ~TimelineSemaphore();
        friend class TimelineSemaphoreHandle;
    };

    class TimelineSemaphoreHandle : public TimelineSemaphoreBase
    {
    public:
        EUREKA_DEFAULT_MOVEABLE_COPYABLE(TimelineSemaphoreHandle);
        TimelineSemaphoreHandle(const TimelineSemaphore& owner) 
        {
            _device = owner._device;
            _semaphore = owner._semaphore;
        }
    };

    class CounterSemaphoreBase : protected TimelineSemaphoreBase
    {
    protected:
        std::atomic_uint64_t _value{0};
        CounterSemaphoreBase(vk::Device device) : TimelineSemaphoreBase(device, 0) {}
        ~CounterSemaphoreBase() = default;
        CounterSemaphoreBase() = default;
        CounterSemaphoreBase(CounterSemaphoreBase&& that) noexcept :
            TimelineSemaphoreBase(std::move(that)),
            _value(that._value.load())
        {

        }

        CounterSemaphoreBase(const CounterSemaphoreBase& that) :
            TimelineSemaphoreBase(that),
            _value(that._value.load())
        {

        }
    public:
        using Semaphore::Get;

        uint64_t Increment()
        {
            return _value.fetch_add(1);
        }

        uint64_t IncrementAndSignal()
        {
            auto prev = _value.fetch_add(1);
            TimelineSemaphoreBase::Signal(_value);
            return prev;
        }

        uint64_t Value() const
        {
            return _value;
        }

        void WaitFor(std::chrono::nanoseconds timeout)
        {
            TimelineSemaphoreBase::WaitFor(_value, timeout);
        }

        bool Query()
        {
            return TimelineSemaphoreBase::QueryValue() == _value;
        }
    };

    class CounterSemaphore : public CounterSemaphoreBase
    {
    public:
        EUREKA_DEFAULT_MOVEONLY(CounterSemaphore);
        CounterSemaphore(vk::Device device) : CounterSemaphoreBase(device) {}
        ~CounterSemaphore();

        friend class CounterSemaphoreHandle;
    };


    class CounterSemaphoreHandle
    {
        CounterSemaphore* _owner;
    public:
        EUREKA_DEFAULT_MOVEABLE_COPYABLE(CounterSemaphoreHandle);

        CounterSemaphoreHandle(CounterSemaphore& owner)
            : _owner(&owner)
        {
        }

        ~CounterSemaphoreHandle() = default;

        vk::Semaphore Get() const
        {
            return _owner->Get();
        }
        uint64_t Increment()
        {
            return _owner->Increment();
        }

        uint64_t IncrementAndSignal()
        {
            return _owner->IncrementAndSignal();
        }

        uint64_t Value() const
        {
            return _owner->Value();
        }

        void WaitFor(std::chrono::nanoseconds timeout)
        {
            return _owner->WaitFor(timeout);
        }

        bool Query()
        {
            return _owner->Query();
        }
    };

    class BinarySemaphoreBase : public Semaphore
    {
    public:
        EUREKA_DEFAULT_MOVEABLE_COPYABLE(BinarySemaphoreBase);
        BinarySemaphoreBase(vk::Device device);
        ~BinarySemaphoreBase();
    };

    class BinarySemaphore : public BinarySemaphoreBase
    {
    public:
        EUREKA_DEFAULT_MOVEONLY(BinarySemaphore);
        BinarySemaphore(vk::Device device) : BinarySemaphoreBase(device) {};
        ~BinarySemaphore();

        friend class BinarySemaphoreHandle;
    };

    class BinarySemaphoreHandle : public TimelineSemaphoreBase
    {
    public:
        EUREKA_DEFAULT_MOVEABLE_COPYABLE(BinarySemaphoreHandle);
        BinarySemaphoreHandle(const BinarySemaphore& owner)
        {
            _device = owner._device;
            _semaphore = owner._semaphore;
        }
    };
}

