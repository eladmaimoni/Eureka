#include "GrpcContext.hpp"
#include <debugger_trace.hpp>



namespace eureka::rpc
{
    constexpr std::size_t GRPC_CONTEXT_POOL_SIZE = 512;

    struct GrpcCompletion
    {
        GrpcTag tag{ nullptr };
        bool ok{ false };
    };

    GrpcContext::GrpcContext(
        std::shared_ptr<grpc::ServerCompletionQueue> completionQueue
    ) 
        :
        _pktsPool(GRPC_CONTEXT_POOL_SIZE),
        _completionQueue(std::move(completionQueue))
    {

    }

    GrpcContext::~GrpcContext()
    {
        Shutdown();

    }


    bool GrpcContext::DoAsyncNext(gpr_timespec& next_call_deadline, std::size_t& count)
    {
        bool active = true;
        GrpcCompletion completion{};

        auto nextStatus = grpc::CompletionQueue::GOT_EVENT;
        
        
        while (nextStatus == grpc::CompletionQueue::GOT_EVENT)
        {
            nextStatus = _completionQueue->AsyncNext(&completion.tag, &completion.ok, next_call_deadline);

            if (nextStatus == grpc::CompletionQueue::GOT_EVENT)
            {
                auto pkt = static_cast<CompletionPacket*>(completion.tag);
                pkt->status = completion.ok;
 
                std::scoped_lock lk(_mtx);
                if (pkt->strand_id)
                {
                    auto strandId = *(pkt->strand_id);
                    _pendingStrands.insert(strandId);
                    _strands.at(strandId)->Enqueue(pkt);
                }
                else
                {
                    _pendingCompletions.emplace_back(pkt);
                }
            
            }
            else if (nextStatus == grpc::CompletionQueue::SHUTDOWN)
            {
                active = false;
                break;
            }
  
            // as small HACK to drain the queue first before we invoke the completion handlers
            // see https://github.com/grpc/grpc/issues/31398
            next_call_deadline = gpr_now(gpr_clock_type::GPR_CLOCK_REALTIME);
        }

        std::unique_lock ulk(_mtx);
        while (!_pendingCompletions.empty())
        {
            auto pkt = std::move(_pendingCompletions.back());
            _pendingCompletions.pop_back();
            ulk.unlock();
            pkt->completion_handler(pkt->status);
            _pktsPool.Deallocate(pkt);
            ++count;
            ulk.lock();
        }

        while (!_pendingStrands.empty())
        {
            auto strandId = _pendingStrands.extract(_pendingStrands.begin()).value();
            ulk.unlock();
            count += _strands.at(strandId)->TryRun();
            ulk.lock();
        }

        static constexpr uint64_t STRAND_GARBAGE_COLLECTION_FREQ = 1000;

        _totalCompletions += count;
        _completionsSinceLastStrandCleanup += count;


        if (_completionsSinceLastStrandCleanup > STRAND_GARBAGE_COLLECTION_FREQ)
        {
            _completionsSinceLastStrandCleanup = 0;
            CleanUnusedStrandsUnderLock();
        }

        return active;
    }

    std::size_t GrpcContext::RunFor(std::chrono::nanoseconds timeout)
    {
        auto grpc_now = gpr_now(gpr_clock_type::GPR_CLOCK_REALTIME);
        auto grpc_duration = gpr_time_from_nanos(timeout.count(), gpr_clock_type::GPR_TIMESPAN);
        auto grpc_deadline = gpr_time_add(grpc_now, grpc_duration);

        std::size_t count = 0;

     
        while (gpr_time_cmp(grpc_now, grpc_deadline) < 0 && DoAsyncNext(grpc_deadline, count))
        {
            grpc_now = gpr_now(gpr_clock_type::GPR_CLOCK_REALTIME);
        }

        return count;
    }

    grpc::ServerCompletionQueue* GrpcContext::Get()
    {
        return _completionQueue.get();
    }

    std::size_t GrpcContext::Run()
    {
        std::size_t count = 0;

        auto inf = gpr_time_from_millis(INT64_MAX, gpr_clock_type::GPR_CLOCK_REALTIME);

        bool active = true;

        while (active)
        {
            active = DoAsyncNext(inf, count);
        }

        return count;
    }

    GrpcTag GrpcContext::CreateTag(CompletionHandler completionHandler, std::optional<uint64_t> strandId)
    {
        auto ptr = _pktsPool.Allocate();
        ptr->completion_handler = std::move(completionHandler);
        ptr->strand_id = strandId;
        return ptr;
    }

    GrpcTag GrpcContext::CreateAsyncNotifyWhenDoneTag(CompletionHandler completionHandler, std::optional<uint64_t> strandId /*= std::nullopt*/)
    {
        auto ptr = _pktsPool.Allocate();
        ptr->completion_handler = std::move(completionHandler);
        ptr->strand_id = strandId;
        return ptr;
    }
    std::shared_ptr<Strand> GrpcContext::CreateStrand()
    {
        //auto strand = std::make_shared<Strand>(_strandIds.fetch_add(1));
        auto strand = Strand::MakeSharedStrand(_strandIds.fetch_add(1));
        std::scoped_lock lk(_mtx);
        _strands.emplace(strand->Id(), strand);
        return strand;
    }

    void GrpcContext::CleanUnusedStrandsUnderLock()
    {
        for (auto itr = _strands.begin(); itr != _strands.end(); ++itr)
        {
            auto [id, strand] = *itr;
            if (strand.use_count() == 1) // strands are unused
            {
                _strands.erase(itr);
            }
        }
    }

    void GrpcContext::Shutdown()
    {
        bool expected = false;
        if (_shutdown.compare_exchange_strong(expected, true))
        {
            // shutdown and drain the completion queue
            _completionQueue->Shutdown();
            Run();

            // leftover tags are rpcs whos completion hasn't been invoked. such as the buggy AsyncNotifyOnStateChange
            DEBUGGER_TRACE("LEFTOVER TAGS = {}", _pktsPool.AllocationsCount());
        }

    }





}

