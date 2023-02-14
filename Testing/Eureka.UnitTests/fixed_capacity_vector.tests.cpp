#include <catch.hpp>
#include <debugger_trace.hpp>
#include <windows.h>
#include <boost/pool/object_pool.hpp>
#include <boost/lockfree/stack.hpp>
#include <stack>

class spinlock
{
    std::atomic_flag _locked{};
public:
    void lock() 
    {
        unsigned counter = 0;
        while (_locked.test_and_set(std::memory_order_acquire))
        { 
            ++counter;
            if (0 == counter % 100)
            {
                std::this_thread::yield();
            }
        }
    }
    void unlock() 
    {
        _locked.clear(std::memory_order_release);
    }
};

using handler_t = std::function<void(bool)>;
struct CompletionPacket
{
    std::size_t key;
    bool status = false;
    bool completed = false;
    handler_t handler;
};

const std::size_t ALLOCATIONS = 100;
const std::size_t TAGS = 1000;


template<typename T>
class FixedObjectPool
{
    std::vector<T>   _pool;
    boost::lockfree::stack<T*>  _addresses;
    //std::atomic_uint64_t         _counter = 0; // stack index
public:
    FixedObjectPool(std::size_t size)
        : _pool(size), _addresses(size)
    {
        for (auto i = 0u; i < size; ++i)
        {
            _addresses.push(&_pool[i]);
            //_addresses[i] = &_pool[i];
        }
    }

    T* allocate()
    {
        T* ptr = nullptr;
        auto res = _addresses.pop(ptr);
        //auto idx = _counter.fetch_add(1, std::memory_order_acquire);

        if (!res)
        {
            // if you reached here:
            // 1. you might have a leak as this means you have used all the objects in the pool
            // 2. you need a larger pool.
            throw std::bad_alloc();
        }
        return ptr;
    }

    void deallocate(T* ptr)
    {
        _addresses.push(ptr);
        //auto idx = _counter.fetch_sub(1, std::memory_order_release);
        //_addresses[idx] = ptr;
    }


};

template<typename T>
class FixedObjectPool2
{
    spinlock         _mtx;
    std::vector<T>   _pool;
    std::stack<T*>   _addresses;

public:
    FixedObjectPool2(std::size_t size)
        : _pool(size)
    {
        for (auto i = 0u; i < size; ++i)
        {
            _addresses.push(&_pool[i]);
        }
    }

    T* allocate()
    {
        std::scoped_lock lk(_mtx);
        T* ptr = _addresses.top();
        _addresses.pop();
        return ptr;
    }

    void deallocate(T* ptr)
    {
        std::scoped_lock lk(_mtx);
        _addresses.push(ptr);
        //auto idx = _counter.fetch_sub(1, std::memory_order_release);
        //_addresses[idx] = ptr;
    }


};

class GrpcContextReused
{
    std::mutex _mtx;
    std::size_t _tagsCount = 0;
    std::atomic_uint64_t _tagsCount2 = 0;

    boost::object_pool<CompletionPacket> _pkts;
    std::unordered_map<uint64_t, CompletionPacket> _tags;
    std::vector<uint64_t> _freeTags;

public:
    GrpcContextReused()
    {
        _freeTags.reserve(ALLOCATIONS);   
        while (_tagsCount < ALLOCATIONS)
        {
            auto key = _tagsCount;
            ++_tagsCount;
            CompletionPacket pkt{};
            pkt.completed = true;
            pkt.key = key;
            auto [itr, b] = _tags.emplace(key, std::move(pkt));
            _freeTags.emplace_back(key);
        }
        
    }

    void* AllocateTag(handler_t handler)
    {
        std::unique_lock lk(_mtx);
        if (!_freeTags.empty())
        {
            auto key = _freeTags.back();
            _freeTags.pop_back();
     
            auto& pkt = _tags.at(key);
            assert(pkt.completed == true);

            lk.unlock();
            pkt.completed = false;
            pkt.handler = std::move(handler);
            return (void*)key;
        }

        auto key = _tagsCount;
        ++_tagsCount;
        
        auto [itr, b] = _tags.emplace(key, CompletionPacket{});
        itr->second.key = key;

        assert(b);

        return (void*)(itr->first);
    }

    void CompleteTag(void* tag)
    {
        uint64_t key = (uint64_t)tag;
        std::unique_lock lk(_mtx);
        auto& pkt = _tags.at(key);
        lk.unlock();
        assert(pkt.completed == false);
        pkt.handler(true);
        pkt.completed = true;

        lk.lock();
        _freeTags.emplace_back(key);
        
    }

};


class GrpcContextNoReuse
{
public:
    void* AllocateTag(handler_t handler)
    {
        auto ptr = new CompletionPacket;
        ptr->handler = std::move(handler);
        return ptr;
    }

    void CompleteTag(void* tag)
    {
        auto pkt = (CompletionPacket*)tag;
        pkt->handler(true);
        pkt->completed = true;
        delete pkt;
    }
};

TEST_CASE("allocation reuse", "[vulkan]")
{
    GrpcContextReused reuser;
    GrpcContextNoReuse nonReuser;

    std::vector<void*> tags;
    tags.reserve(TAGS);

    BENCHMARK("allocation reuse")
    {
        for (auto i = 0u; i < TAGS; ++i)
        {
            auto tag = reuser.AllocateTag([](bool) {});
            reuser.CompleteTag(tag);
        }
    };


    BENCHMARK("no allocation reuse")
    {
        for (auto i = 0u; i < TAGS; ++i)
        {
            auto tag = nonReuser.AllocateTag([](bool) {});
            nonReuser.CompleteTag(tag);
        }
    };

}

std::size_t TOTAL_ITERATIONS = 1000;
std::size_t ALLOCATIONS_PER_ITER = 100;


TEST_CASE("FixedObjectPool", "[vulkan]")
{
    FixedObjectPool2<CompletionPacket> pool(ALLOCATIONS_PER_ITER);

    std::vector<CompletionPacket*> objects;
    objects.reserve(ALLOCATIONS_PER_ITER);

    BENCHMARK("pool")
    {
        for (auto i = 0; i < TOTAL_ITERATIONS; ++i)
        {
            for (auto j = 0; j < ALLOCATIONS_PER_ITER; ++j)
            {
                auto ptr = pool.allocate();
                objects.emplace_back(ptr);
            }
            for (auto j = 0; j < ALLOCATIONS_PER_ITER; ++j)
            {
                auto ptr = objects.back();
                objects.pop_back();
                pool.deallocate(ptr);
            }
        }
    };


    BENCHMARK("new-delete")
    {
        for (auto i = 0; i < TOTAL_ITERATIONS; ++i)
        {
            for (auto j = 0; j < ALLOCATIONS_PER_ITER; ++j)
            {
                objects.emplace_back(new CompletionPacket);
            }
            for (auto j = 0; j < ALLOCATIONS_PER_ITER; ++j)
            {
                auto ptr = objects.back();
                objects.pop_back();
                delete ptr;
            }
        }
    };

}