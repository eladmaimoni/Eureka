#include "VisualizationService.hpp"
#include "StreamHandlers.hpp"
#include "UnaryHandlers.hpp"
#include "GrpcContext.hpp"
#include <profiling.hpp>
#include <debugger_trace.hpp>

namespace eureka::rpc
{
    VisualizationService::VisualizationService(
        std::shared_ptr<rgoproto::LiveSlamUIService::AsyncService> service,
        std::shared_ptr<GrpcContext> grpcContext
    ) :
        _service(std::move(service)),
        _grpcContext(std::move(grpcContext)),
        _poseGraphStreamingHandler(PoseGraphStreamingHandler::Make(_service, _grpcContext)),
        _realtimePoseStreamingHandler(RealtimePoseStreamingHandler::Make(_service, _grpcContext)),
        _forceFullGPOHandler(ForceFullGPOHandler::Make(_service, _grpcContext))
    {

    }

    VisualizationService::~VisualizationService()
    {
        // stop all active operations
        _poseGraphStreamingHandler->Shutdown();
        _realtimePoseStreamingHandler->Shutdown();

        DEBUGGER_TRACE("Service Dtor");
    }

    void VisualizationService::Start()
    {
        bool expected = false;
        if (_active.compare_exchange_strong(expected, true))
        {
            DEBUGGER_TRACE("START Service");

            _poseGraphStreamingHandler->Start();
            _realtimePoseStreamingHandler->Start();
            _forceFullGPOHandler->Start(
                [this]
                (const rgoproto::ForceFullGPORequestMsg& /*request*/, rgoproto::ForceFullGPOResponseMsg& /*response*/)
                {
                    DEBUGGER_TRACE("force gpo optimization");
                }
            );
        }
    }

    void VisualizationService::Stop()
    {
        bool expected = true;
        if (_active.compare_exchange_strong(expected, false))
        {
            _poseGraphStreamingHandler->Stop();
            _realtimePoseStreamingHandler->Stop();
            _forceFullGPOHandler->Stop();
        }
    }

    std::shared_ptr<rgoproto::PoseGraphStreamingMsg> VisualizationService::ExchangeData(std::shared_ptr<rgoproto::PoseGraphStreamingMsg> msg)
    {
        return _poseGraphStreamingHandler->ExchangeData(std::move(msg));
    }

    std::shared_ptr<rgoproto::RealtimePoseStreamingMsg> VisualizationService::ExchangeData(std::shared_ptr<rgoproto::RealtimePoseStreamingMsg> msg)
    {
        return _realtimePoseStreamingHandler->ExchangeData(std::move(msg));
    }
}

