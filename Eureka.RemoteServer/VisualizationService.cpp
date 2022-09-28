#include "VisualizationService.hpp"

namespace rgo
{
    //
    // https://stackoverflow.com/questions/70692739/detect-client-context-destruction-from-grpc-server
    //

    VisualizationServer::VisualizationServer() :
        _completionQueue(_serverBuilder.AddCompletionQueue()),
        _poseGraphStreamingHandler(&_service, _completionQueue.get())
    {

    }

    VisualizationServer::~VisualizationServer()
    {
        Stop();
    }

    void VisualizationServer::Start(std::string thisServerListingEndpoint)
    {
        if (!_active)
        {
            _serverBuilder.AddListeningPort(thisServerListingEndpoint, grpc::InsecureServerCredentials());
            _serverBuilder.RegisterService(&_service);
            _grpcServer = _serverBuilder.BuildAndStart();


            if (!_grpcServer)
            {
                DEBUGGER_TRACE("failed creating server - perhaps server already active");
                throw std::runtime_error("failed creating server - perhaps server already active");
            }

            DEBUGGER_TRACE("START SERVER");


            _poseGraphStreamingHandler.Start();
            _active = true;
        }
    }

    void VisualizationServer::Stop()
    {
        if (_active)
        {
            _poseGraphStreamingHandler.Stop();
            _grpcServer->Shutdown();

            DEBUGGER_TRACE("SHUTDOWN SERVER");

   
            _active = false;
        }
    }

    void VisualizationServer::PollCompletions(std::chrono::nanoseconds timeout)
    {
        //PROFILE_SCOPE(PROFILING_CATEGORY_DEFAULT, "PollCompletions");
        auto grpc_now = gpr_now(gpr_clock_type::GPR_CLOCK_REALTIME);
        auto grpc_duration = gpr_time_from_nanos(timeout.count(), gpr_clock_type::GPR_TIMESPAN);
        auto grpc_deadline = gpr_time_add(grpc_now, grpc_duration);


        while (gpr_time_cmp(grpc_now, grpc_deadline) < 0)
        {
            bool ok = false;
            GrpcTag tag;
            auto nextStatus = _completionQueue->AsyncNext(&tag, &ok, grpc_deadline);
      

            if (nextStatus == grpc::CompletionQueue::GOT_EVENT)
            {
                CallHandler(tag, ok);
            }
            else
            {

            }


            grpc_now = gpr_now(gpr_clock_type::GPR_CLOCK_REALTIME);
        }
    }


    void VisualizationServer::RunCompletions()
    {
        bool ok = false;
        GrpcTag tag;
        while (_completionQueue->Next(&tag, &ok))
        {
            CallHandler(tag, ok);
        }
    }

    void VisualizationServer::CallHandler(GrpcTag tag, bool ok)
    {
        //CompletionTag completionTag(tag);
        _poseGraphStreamingHandler.Proceed(tag, ok);
        //switch (completionTag.type)
        //{
        //case CompletionType::PoseGraphStreaming:
    
        //    break;
        //default:
        //    DEBUGGER_TRACE("unknown completion type");
        //    break;
        //}

        //if (!ok)
        //{
        //    DEBUGGER_TRACE("CallHandler got ok == false");

        //}
        //else
        //{

        //}

        //std::string prefix("Hello ");
        //reply_.set_message(prefix + request_.name() + ", no " + request_.num_greetings());

        // For illustrating the queue-to-front behaviour
        //using namespace std::chrono_literals;
        //std::this_thread::sleep_for(1s);
    }


    void PoseGraphStreamingHandler::Listen()
    {
        assert(_state == HandlerState::Listening);
        _packetsNum = 0;
        //CompletionTag cancel_tag((void*)0xFF, CompletionType::PoseGraphStreaming);
        _serverContext = std::make_unique<grpc::ServerContext>();
        _serverContext->AsyncNotifyWhenDone((void*)0xFF);
        //_serverContext->tr
        //auto clientContext = grpc::ClientContext::FromServerContext(*_serverContext);
        //clientContext->
        _asyncWriter = std::make_unique<grpc::ServerAsyncWriter<eureka::PoseGraphVisualizationUpdateMsg>>(_serverContext.get());

        DEBUGGER_TRACE("SERVER Starts POSE GRAPH STREAMING");
        //CompletionTag stream_tag((void*)0x11, CompletionType::PoseGraphStreaming);
        _service->RequestStartPoseGraphStreaming(
            _serverContext.get(),
            &_clientRequest,
            _asyncWriter.get(),
            _completionQueue,
            _completionQueue,
            (void*)0x11
        );

        _state = HandlerState::Streaming;
    }


    void PoseGraphStreamingHandler::Proceed(void* tag, bool ok)
    {
        if (_serverContext->IsCancelled())
        {
            DEBUGGER_TRACE("FUCKING CANCEL");
        }
        if (ok)
        {
            if (tag == (void*)0xFF)
            {
                //if (_serverContext->IsCancelled())
                // https://grpc.io/blog/deadlines/
                // https://stackoverflow.com/questions/70692739/detect-client-context-destruction-from-grpc-server
                // https://stackoverflow.com/questions/64297617/grpc-c-how-to-detect-client-disconnected-in-async-server/64379571#64379571
                // https://github.com/grpc/grpc/issues/22839
                grpc::Status status(grpc::CANCELLED, "cancelled");

                CompletionTag finishTag(FINISH_CODE, CompletionType::PoseGraphStreaming);

                if (_state == HandlerState::Streaming)
                {
                    _asyncWriter->Finish(status, (void*)0x22);
                    _state = HandlerState::Inactive;

                    DEBUGGER_TRACE("server finished {} total packets write = {}", status.error_message(), _packetsNum);
                }

                //Listen();
                DEBUGGER_TRACE("client cancelled");
            }
            else if (tag == (void*)0x11)
            {
                if (_packetsNum == 0)
                {
                    DEBUGGER_TRACE("server first packet write");
                }
                std::this_thread::sleep_for(100ms);
                eureka::PoseGraphVisualizationUpdateMsg poseGraphUpdate;
                std::vector<float> dummy(5, 42);

                poseGraphUpdate.poses();
                poseGraphUpdate.mutable_poses()->Assign(dummy.begin(), dummy.end());

                //CompletionTag tag(STREAM_CODE, CompletionType::PoseGraphStreaming);

                _asyncWriter->Write(poseGraphUpdate, (void*)0x11);

                ++_packetsNum;

                //if (_packetsNum % 1000 == 0)
                {
                    //DEBUGGER_TRACE("Write");
                }
            }
            else if (tag == (void*)0x22)
            {
                DEBUGGER_TRACE("FINISH_CODE ");
            }
            else
            {
                DEBUGGER_TRACE("WTF");
            }

        }
        else
        {


            DEBUGGER_TRACE("CallHandler got ok == false total packets write = {}", _packetsNum);
            // connection was terminated, but we are still alive
            _state = HandlerState::Listening;
            //Listen();
        }
    }

}

