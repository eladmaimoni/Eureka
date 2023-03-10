#pragma once
#include <compiler.hpp>
#include <unordered_set>
#include <stack>
#include <atomic>
#include <debugger_trace.hpp>

using namespace std::chrono_literals;
EUREKA_MSVC_WARNING_PUSH
EUREKA_MSVC_WARNING_DISABLE(4702 4127)
#include <grpcpp/server_builder.h>
#include <grpcpp/alarm.h>
EUREKA_MSVC_WARNING_POP

namespace eureka::rpc
{
    using GrpcTag = void*;

    template<unsigned SPINCOUNT = 100>
    class spinlock_t
    {
        std::atomic_flag _locked{};
    public:
        void lock()
        {
            unsigned counter = 0;
            while (_locked.test_and_set(std::memory_order_acquire))
            {
                ++counter;
                if (0 == counter % SPINCOUNT)
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

    using spinlock = spinlock_t<100>;

    template<typename T>
    class FixedObjectPool
    {
        /*
        fast, very simple, thread-safe replacement for new / delete
        limitations:
        - fixed maximum amount of allocations
        - single object type
        */
        spinlock         _mtx;
        std::vector<T>   _pool;
        std::stack<T*>   _addresses;
    public:
        FixedObjectPool(std::size_t size)
            : _pool(size)
        {
            for (auto i = 0u; i < size; ++i)
            {
                _addresses.push(&_pool[i]);
            }
        }
        ~FixedObjectPool()
        {
            DEBUGGER_TRACE("release pool");
        }
        T* Allocate()
        {
            std::scoped_lock lk(_mtx);
            T* ptr = _addresses.top();
            _addresses.pop();
            return ptr;

        }

        void Deallocate(T* ptr)
        {
            std::scoped_lock lk(_mtx);
            _addresses.push(ptr);
        }

        uint64_t Size()
        {
            return _pool.size();
        }

        uint64_t AllocationsCount()
        {
            std::scoped_lock lk(_mtx);
            return  _pool.size() - _addresses.size();
        }


    };


    struct CompletionPacket
    {
        std::function<void(bool)> completion_handler;
        std::optional<uint64_t> strand_id{ std::nullopt };
        bool status{ false };
    };


    class Strand
    {
        std::mutex _mtx;
        uint64_t _id{ 0 };
        std::vector<CompletionPacket*> _completionPkts;
        std::atomic_bool _running{ false };
    public:
        uint64_t Id() const
        {
            return 0;
        }

    private:
        friend class GrpcContext;
        bool Empty()
        {
            std::scoped_lock lk(_mtx);
            return _completionPkts.empty();
        }


        Strand(uint64_t id)
            : _id(id)
        {

        }
        ~Strand()
        {
            assert(_completionPkts.empty());
        }
        static std::shared_ptr<Strand> MakeSharedStrand(uint64_t id)
        {
            // https://stackoverflow.com/questions/8147027/how-do-i-call-stdmake-shared-on-a-class-with-only-protected-or-private-const

            class make_shared_enabler : public Strand 
            {
            public: make_shared_enabler(uint64_t id) : Strand(id) {}         
            };

            return std::make_shared<make_shared_enabler>(id);
        }
            
        void Enqueue(CompletionPacket* pkt)
        {
            assert(pkt->strand_id == Id());
            std::scoped_lock lk(_mtx);
            _completionPkts.emplace_back(pkt);
        }

        std::size_t TryRun()
        {
            std::size_t count = 0;
            bool expected = false;
            if (_running.compare_exchange_strong(expected, true)) // HERE
            {  
                // HERE Enqueue POST
                std::unique_lock lk(_mtx);
        
                while (!_completionPkts.empty())
                {
                    auto pkt = _completionPkts.back();
                    _completionPkts.pop_back();
                    lk.unlock();
                    // HERE Enqueue POST
                    pkt->completion_handler(pkt->status);
                    delete pkt;
                    ++count;

                    lk.lock();
                }  
   
                _running.store(false);
            }

            // HERE Enqueue POST

            return count;
        }
    };


    using CompletionHandler = std::function<void(bool)>;


    class GrpcContext
    {
        //
        // GrpcContext - think asio::io_context, or asio-grpc agrpc::GrpcContext
        //
    private:
        std::mutex                                            _mtx;
        FixedObjectPool<CompletionPacket>                     _pktsPool;
        std::atomic_bool                                      _shutdown = false;
        std::atomic_uint64_t                                  _strandIds = 0;
        std::vector<CompletionPacket*>                        _pendingCompletions;
        std::unordered_set<uint64_t>                          _pendingStrands;
        std::unordered_map<uint64_t, std::shared_ptr<Strand>> _strands;
        uint64_t                                              _totalCompletions = 0;
        uint64_t                                              _completionsSinceLastStrandCleanup = 0;
        std::shared_ptr<grpc::ServerCompletionQueue>          _completionQueue;

        bool DoAsyncNext(gpr_timespec& next_call_deadline, std::size_t& count);

        void CleanUnusedStrandsUnderLock();

    public:
        GrpcContext(std::shared_ptr<grpc::ServerCompletionQueue> completionQueue);
        ~GrpcContext();

        void Shutdown();

        GrpcContext(const GrpcContext&) = delete;
        GrpcContext& operator=(const GrpcContext&) = delete;

        //
        // Thread Safe, asio::io_context-like mechanism. may be called from one or more threads
        //
        std::size_t RunFor(std::chrono::nanoseconds timeout);
        std::size_t Run();
    
        //
        // Get the raw completion queue for RPC calls
        //
        grpc::ServerCompletionQueue* Get();

        //
        // CreateTag - Create a new tag for a grpc call
        // completion handlers that should synchronize with each other should use the strand mechanism.
        // Note that grpc::ServerContext::AsyncNotifyWhenDone implementation has a BUG that causes 
        // completion tags to be 'lost' when the server is shut down, current workaround is to manually track them
        // 
        GrpcTag CreateTag(CompletionHandler completionHandler, std::optional<uint64_t> strandId = std::nullopt);
        GrpcTag CreateAsyncNotifyWhenDoneTag(CompletionHandler completionHandler, std::optional<uint64_t> strandId = std::nullopt);

        std::shared_ptr<Strand> CreateStrand();  
    

    };

    

}

