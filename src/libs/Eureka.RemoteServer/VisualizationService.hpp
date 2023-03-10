#pragma once
#include <compiler.hpp>
EUREKA_MSVC_WARNING_PUSH
EUREKA_MSVC_WARNING_DISABLE(4702 4127)
#include <proto/rgorpc.grpc.pb.h>
EUREKA_MSVC_WARNING_POP


using namespace std::chrono_literals;

namespace rgo
{
    //class GPOCoordinator;
}

namespace eureka::rpc
{
    class GrpcContext;
    struct GPOStreamPolicy;
    struct RealtimePoseStreamPolicy;
    struct ForceFullGPORPCPolicy;

    template<typename StreamPolicy> class GenericServerToClientStreamer;
    template<typename RPCPolicy> class GenericImmediateUnaryRPC;

    using PoseGraphStreamingHandler = GenericServerToClientStreamer<GPOStreamPolicy>;
    using RealtimePoseStreamingHandler = GenericServerToClientStreamer<RealtimePoseStreamPolicy>;
    using ForceFullGPOHandler = GenericImmediateUnaryRPC<ForceFullGPORPCPolicy>;

    class VisualizationService
    {
        std::atomic_bool                                               _active{ false };
        std::shared_ptr<GrpcContext>                                   _grpcContext;
        std::shared_ptr<rgoproto::LiveSlamUIService::AsyncService>     _service;
        std::shared_ptr<PoseGraphStreamingHandler>                     _poseGraphStreamingHandler;
        std::shared_ptr<RealtimePoseStreamingHandler>                  _realtimePoseStreamingHandler;
        std::shared_ptr<ForceFullGPOHandler>                           _forceFullGPOHandler;
    public:
        VisualizationService(std::shared_ptr<rgoproto::LiveSlamUIService::AsyncService> service, std::shared_ptr<GrpcContext> grpcContext);
        ~VisualizationService();

        //
        // public thread safe functions
        //
        void Start();
        void Stop();
        std::shared_ptr<rgoproto::PoseGraphStreamingMsg> ExchangeData(std::shared_ptr<rgoproto::PoseGraphStreamingMsg> msg);
        std::shared_ptr<rgoproto::RealtimePoseStreamingMsg> ExchangeData(std::shared_ptr<rgoproto::RealtimePoseStreamingMsg> msg);
        
    };

}

