#include <compiler.hpp>
#include "RemoteUIServer.hpp"
#include <grpcpp/create_channel.h>
#include <logging.hpp>
#include <agrpc/asio_grpc.hpp>
#include <asio/detached.hpp>
#include <asio/bind_executor.hpp>
#include <asio/coroutine.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/experimental/awaitable_operators.hpp>
using namespace std::literals::chrono_literals;


void TestCancellationRawGRPC()
{
    const auto server_grpc_port = "50051";
    const auto server_host = std::string("0.0.0.0:") + server_grpc_port;
    const auto client_host = std::string("localhost:") + server_grpc_port;

    constexpr uint64_t SERVER_CANCELLATION_TAG = 111;
    constexpr uint64_t SERVER_STREAMING_TAG = 222;
    constexpr uint64_t CLIENT_STREAMING_TAG = 333;
    //inline constexpr uint64_t CLIENT_STREAMING_TAG = 444;
    // build server
    grpc::ServerBuilder  serverBuilder;
    std::unique_ptr<grpc::ServerCompletionQueue> serverCompletionQueue = serverBuilder.AddCompletionQueue();
    eureka::LiveSlamControlCenter::AsyncService  service;
    serverBuilder.AddListeningPort(server_host, grpc::InsecureServerCredentials());
    serverBuilder.RegisterService(&service);
    std::unique_ptr<grpc::Server> grpcServer = serverBuilder.BuildAndStart();
    assert(grpcServer);

    // build client

    grpc::CompletionQueue clientCompletionQueue; 
    eureka::LiveSlamControlCenter::Stub stub(grpc::CreateChannel(client_host, grpc::InsecureChannelCredentials()));


    //
    // server listens to RPC request to start streaming
    // 
    eureka::StartPoseGraphUpdatesMsg  clientRequestServerSide;

    grpc::ServerContext serverContext;
    serverContext.AsyncNotifyWhenDone((void*)SERVER_CANCELLATION_TAG);

    grpc::ServerAsyncWriter<eureka::PoseGraphVisualizationUpdateMsg> writer(&serverContext);


    service.RequestStartPoseGraphStreaming(
        &serverContext,
        &clientRequestServerSide,
        &writer,
        serverCompletionQueue.get(),
        serverCompletionQueue.get(),
        (void*)SERVER_STREAMING_TAG
    );

    //
    // client requests RPC
    //
    grpc::ClientContext clientContext;
    eureka::StartPoseGraphUpdatesMsg clientRequestClientSide;
    clientRequestClientSide.set_integer(42);
    auto reader = stub.PrepareAsyncStartPoseGraphStreaming(&clientContext, clientRequestClientSide, &clientCompletionQueue);
    reader->StartCall((void*)CLIENT_STREAMING_TAG);

    void* clientTag{};
    void* serverTag{};
    bool clientOk = false;
    bool serverOk = false;
    clientCompletionQueue.Next(&clientTag, &clientOk);
    serverCompletionQueue->Next(&serverTag, &serverOk);
    assert(clientTag == (void*)CLIENT_STREAMING_TAG);
    assert(clientOk);
    assert(serverTag == (void*)SERVER_STREAMING_TAG);
    assert(serverOk);

    // client read + server write
    eureka::PoseGraphVisualizationUpdateMsg msgClientSide;
    eureka::PoseGraphVisualizationUpdateMsg msgServerSide;
    reader->Read(&msgClientSide, (void*)CLIENT_STREAMING_TAG);
    writer.Write(msgServerSide, (void*)SERVER_STREAMING_TAG);

    clientCompletionQueue.Next(&clientTag, &clientOk);
    serverCompletionQueue->Next(&serverTag, &serverOk);
    assert(clientTag == (void*)CLIENT_STREAMING_TAG);
    assert(clientOk);
    assert(serverTag == (void*)SERVER_STREAMING_TAG);
    assert(serverOk);

    // client cancel
    writer.Write(msgServerSide, (void*)SERVER_STREAMING_TAG); serverCompletionQueue->Next(&serverTag, &serverOk);
    writer.Write(msgServerSide, (void*)SERVER_STREAMING_TAG); serverCompletionQueue->Next(&serverTag, &serverOk);
    writer.Write(msgServerSide, (void*)SERVER_STREAMING_TAG); serverCompletionQueue->Next(&serverTag, &serverOk);
    writer.Write(msgServerSide, (void*)SERVER_STREAMING_TAG); serverCompletionQueue->Next(&serverTag, &serverOk);



    clientContext.TryCancel();
    serverCompletionQueue->Next(&serverTag, &serverOk);


    assert(serverTag == (void*)SERVER_CANCELLATION_TAG);
    assert(serverOk);
}

void TestCancellationWithAsioGRPCClient()
{
    const auto server_grpc_port = "50051";
    const auto server_host = std::string("0.0.0.0:") + server_grpc_port;
    const auto client_host = std::string("localhost:") + server_grpc_port;

    constexpr uint64_t SERVER_CANCELLATION_TAG = 111;
    constexpr uint64_t SERVER_STREAMING_TAG = 222;


    // build server
    grpc::ServerBuilder  serverBuilder;
    std::unique_ptr<grpc::ServerCompletionQueue> serverCompletionQueue = serverBuilder.AddCompletionQueue();
    eureka::LiveSlamControlCenter::AsyncService  service;
    serverBuilder.AddListeningPort(server_host, grpc::InsecureServerCredentials());
    serverBuilder.RegisterService(&service);
    std::unique_ptr<grpc::Server> grpcServer = serverBuilder.BuildAndStart();
    assert(grpcServer);

    // build client

    agrpc::GrpcContext clientCompletionQueue(std::make_unique<grpc::CompletionQueue>());
    eureka::LiveSlamControlCenter::Stub stub(grpc::CreateChannel(client_host, grpc::InsecureChannelCredentials()));


    //
    // server listens to RPC request to start streaming
    // 
    eureka::StartPoseGraphUpdatesMsg  clientRequestServerSide;

    grpc::ServerContext serverContext;
    serverContext.AsyncNotifyWhenDone((void*)SERVER_CANCELLATION_TAG);

    grpc::ServerAsyncWriter<eureka::PoseGraphVisualizationUpdateMsg> writer(&serverContext);


    service.RequestStartPoseGraphStreaming(
        &serverContext,
        &clientRequestServerSide,
        &writer,
        serverCompletionQueue.get(),
        serverCompletionQueue.get(),
        (void*)SERVER_STREAMING_TAG
    );

    //
    // client requests RPC
    //
    grpc::ClientContext clientContext;
    eureka::StartPoseGraphUpdatesMsg clientRequestClientSide;
    clientRequestClientSide.set_integer(42);
    std::unique_ptr<grpc::ClientAsyncReader<eureka::PoseGraphVisualizationUpdateMsg>> clientReader;
    asio::co_spawn(clientCompletionQueue,
        [&]() -> asio::awaitable<void>
        {
            auto [reader, ok] = co_await agrpc::request(
                &eureka::LiveSlamControlCenter::Stub::PrepareAsyncStartPoseGraphStreaming,
                stub,
                clientContext,
                clientRequestClientSide,
                asio::bind_executor(clientCompletionQueue, asio::use_awaitable)
            );
            clientReader = std::move(reader);
            assert(ok);
        },
        asio::detached
   );


 
  
    void* serverTag{};
    bool serverOk = false;
    bool clientRanOk = false;
    clientRanOk = clientCompletionQueue.run();
    serverCompletionQueue->Next(&serverTag, &serverOk);
    assert(clientRanOk);
    assert(serverTag == (void*)SERVER_STREAMING_TAG);
    assert(serverOk);

     //client read + server write
    eureka::PoseGraphVisualizationUpdateMsg msgClientSide;
    eureka::PoseGraphVisualizationUpdateMsg msgServerSide;
    asio::co_spawn(clientCompletionQueue,
        [&]() -> asio::awaitable<void>
        {
            bool readOk = co_await agrpc::read(clientReader, msgClientSide, asio::bind_executor(clientCompletionQueue, asio::use_awaitable));
            assert(readOk);
        },
        asio::detached
            );
    writer.Write(msgServerSide, (void*)SERVER_STREAMING_TAG);

    clientRanOk = clientCompletionQueue.run();
    serverCompletionQueue->Next(&serverTag, &serverOk);
    assert(clientRanOk);
    assert(serverTag == (void*)SERVER_STREAMING_TAG);
    assert(serverOk);

    // client cancel
    writer.Write(msgServerSide, (void*)SERVER_STREAMING_TAG); serverCompletionQueue->Next(&serverTag, &serverOk);
    writer.Write(msgServerSide, (void*)SERVER_STREAMING_TAG); serverCompletionQueue->Next(&serverTag, &serverOk);
    writer.Write(msgServerSide, (void*)SERVER_STREAMING_TAG); serverCompletionQueue->Next(&serverTag, &serverOk);
    writer.Write(msgServerSide, (void*)SERVER_STREAMING_TAG); serverCompletionQueue->Next(&serverTag, &serverOk);



    clientContext.TryCancel();
    serverCompletionQueue->Next(&serverTag, &serverOk);


    assert(serverTag == (void*)SERVER_CANCELLATION_TAG);
    assert(serverOk);
}

int main()
{
    try
    {
        TestCancellationWithAsioGRPCClient();
        TestCancellationRawGRPC();
        const auto server_grpc_port = "50051";
        const auto host = std::string("0.0.0.0:") + server_grpc_port;
        eureka::RemoteUIServer server;
        server.Start(host);

        std::string line;
        while (true)
        {
            std::getline(std::cin, line);

            if (line == "q")
            {
                break;
            }
            else if (line == "start")
            {
                server.Start(host);
            }
            else if (line == "stop")
            {
                server.Stop();
            }
        }

        CLOG("server quitting ...");
    }
    catch (const std::exception& err)
    {
        CLOG("server error {}", err.what());
    }
    
    CLOG("server main thread exited");

    return 0;
}

