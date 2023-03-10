#include "LiveSlamServer.hpp"
#include <debugger_trace.hpp>
#include "LiveSlamServiceHelpers.hpp"


namespace eureka::rpc
{
    LiveSlamServer::LiveSlamServer(std::vector<std::shared_ptr<grpc::Service>> services) :
        _grpcContext(std::make_shared<GrpcContext>(_builder.AddCompletionQueue())),
        _services(std::move(services))
    {

    }

    LiveSlamServer::~LiveSlamServer()
    {
        Shutdown();

        _grpcContext->Shutdown(); // we must drain the queue before the grpc::Server instance is dead


        DEBUGGER_TRACE("Server Dtor");
    }


    void LiveSlamServer::Start(std::string thisServerListingEndpoint)
    {
        if (!_active)
        {
            _builder.AddListeningPort(thisServerListingEndpoint, grpc::InsecureServerCredentials());
            for (const auto& service : _services)
            {
                _builder.RegisterService(service.get()); // TODO called twice?
            }
            _grpcServer = _builder.BuildAndStart();
       
            if (!_grpcServer)
            {
                DEBUGGER_TRACE("failed creating server - perhaps server already active");
                throw std::runtime_error("failed creating server - perhaps server already active");
            }
            _active = true;
        }

    }

    void LiveSlamServer::Shutdown()
    {
        if (_active)
        {  
            _grpcServer->Shutdown(GrpcTimpointFromNow(10ms));
            //_grpcServer->Shutdown();
            _grpcContext->RunFor(10ms);
            DEBUGGER_TRACE("SHUTDOWN SERVER");
            _active = false;
        }
    }

    std::shared_ptr<GrpcContext> LiveSlamServer::GetContext()
    {
        return _grpcContext;
    }





}

