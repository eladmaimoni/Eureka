#include <catch2/catch.hpp>
#include <debugger_trace.hpp>
#include <VisualizationService.hpp>
#include <RemoteLiveSlamClient.hpp>
#include <grpcpp/create_channel.h>
#include <logging.hpp>

using MyService = eureka::LiveSlamControlCenter;
using StartStreamMsg = eureka::StartPoseGraphUpdatesMsg;
using StreamMsg = eureka::PoseGraphVisualizationUpdateMsg;

class MyStrreamingReactor : public grpc::ServerWriteReactor<StreamMsg>
{
    std::size_t _count = 0;
    void OnDone() override 
    {
        CLOG("OnDone total {}", _count);
        delete this; 
    }

    void OnCancel() override 
    {
        CLOG("OnCancel total {}", _count);
        Finish(grpc::Status::OK);
    }
    

    void OnWriteDone(bool /*ok*/) override 
    { 
        DoWrite(); 
    }

    // we can simply call this whenever we wish
    void DoWrite()
    {
        StreamMsg msg{};
        StartWrite(&msg);
    }

};

class ServiceCallbackImpl : public eureka::LiveSlamControlCenter::CallbackService
{
public:
    grpc::ServerWriteReactor<StreamMsg>* StartPoseGraphStreaming(grpc::CallbackServerContext* context, const StartStreamMsg* request) override
    {
        CLOG("StartPoseGraphStreaming");
        return new MyStrreamingReactor();
    }
};

class ServiceSyncImpl : public eureka::LiveSlamControlCenter::Service
{
    grpc::Status StartPoseGraphStreaming(grpc::ServerContext* context, const eureka::StartPoseGraphUpdatesMsg* request, grpc::ServerWriter<eureka::PoseGraphVisualizationUpdateMsg>* writer)
    {
        eureka::PoseGraphVisualizationUpdateMsg msg{};

        bool writeOk = true;

        std::size_t count = 0;
        while (writeOk)
        {
            writeOk = writer->Write(msg);
            if (count == 0)
            {
                CLOG("server start write");
            }
            ++count;
        }
        CLOG("server write done total = {}", count);
        return grpc::Status::OK;
    }
};


void SyncClient()
{

    const auto server_grpc_port = "50051";
    const auto remoteServerEndpoint = std::string("localhost:") + server_grpc_port;


    auto stub = std::make_unique<MyService::Stub>(grpc::CreateChannel(remoteServerEndpoint, grpc::InsecureChannelCredentials()));

    

    {
        CLOG("client - start read");
        StartStreamMsg request{};
        grpc::ClientContext clientContext;

        auto reader = stub->StartPoseGraphStreaming(&clientContext, request);


        std::size_t count = 0;
        bool readOk = true;
        while (count < 10000 && readOk)
        {
            StreamMsg streamMsg{};
            readOk = reader->Read(&streamMsg);
       

            if (readOk == false)
            {
                CLOG("client - read not ok");
            }
            else
            {
                ++count;
            }
        }
        CLOG("client - end read {} messages, cancelling", count);
        clientContext.TryCancel();


        auto status = reader->Finish();
        CLOG("client - called Finish, status = {}", status.error_message());
    }

    std::this_thread::sleep_for(100s);

}

void AsyncServer()
{
    const auto server_grpc_port = "50051";
    const auto server_host = std::string("0.0.0.0:") + server_grpc_port;

    grpc::ServerBuilder serverBuilder;
    std::unique_ptr<grpc::Server> grpcServer;
    MyService::AsyncService service;
    serverBuilder.AddListeningPort(server_host, grpc::InsecureServerCredentials());
    serverBuilder.RegisterService(&service);
    auto completionQueue = serverBuilder.AddCompletionQueue();
    //grpc::CompletionQueue nextCallQueue;
    grpcServer = serverBuilder.BuildAndStart();

    void* TAG1_START = (void*)1;
    void* TAG2_WRITE = (void*)2;
    void* TAG3_FINISH = (void*)3;
    void* TAG4_CANCEL = (void*)4;

    StartStreamMsg request;
    grpc::ServerContext serverContext;
    grpc::ServerAsyncWriter<StreamMsg> writer(&serverContext);
    serverContext.AsyncNotifyWhenDone(TAG4_CANCEL);
    service.RequestStartPoseGraphStreaming(&serverContext, &request, &writer, completionQueue.get(), completionQueue.get(), TAG1_START);

    CLOG("server start");
    void* tag;
    bool ok;
    std::size_t count{ 0 };
    while (completionQueue->Next(&tag, &ok))
    {
        eureka::PoseGraphVisualizationUpdateMsg streamMsg{};
        if (tag == TAG1_START)
        {
            CLOG("server start stream ok = {}", (int)ok);
            writer.Write(streamMsg, TAG2_WRITE);
            ++count;
        }
        else if (tag == TAG2_WRITE)
        {
            writer.Write(streamMsg, TAG2_WRITE);
            ++count;
        }
        else if (tag == TAG3_FINISH)
        {
            CLOG("server finished stream ok = {} total packets = {}", (int)ok, count);
        }
        else if (tag == TAG4_CANCEL)
        {
            writer.Finish(grpc::Status::CANCELLED, TAG3_FINISH);
            CLOG("server got cancel, ok = {}", (int)ok);
        }
    }


    std::this_thread::sleep_for(100s);
    grpcServer->Shutdown();
    CLOG("server end");


}

void AsyncCallbackServer()
{
    const auto server_grpc_port = "50051";
    const auto server_host = std::string("0.0.0.0:") + server_grpc_port;

    grpc::ServerBuilder serverBuilder;
    std::unique_ptr<grpc::Server> grpcServer;
    ServiceCallbackImpl service;
    serverBuilder.AddListeningPort(server_host, grpc::InsecureServerCredentials());
    serverBuilder.RegisterService(&service);
    grpcServer = serverBuilder.BuildAndStart();

    CLOG("server start");
    grpcServer->Wait();
    CLOG("server end");
}

void SyncServer()
{
    const auto server_grpc_port = "50051";
    const auto server_host = std::string("0.0.0.0:") + server_grpc_port;

    grpc::ServerBuilder serverBuilder;
    std::unique_ptr<grpc::Server> grpcServer;
    ServiceSyncImpl service;
    serverBuilder.AddListeningPort(server_host, grpc::InsecureServerCredentials());
    serverBuilder.RegisterService(&service);
    grpcServer = serverBuilder.BuildAndStart();

    CLOG("server start");
    grpcServer->Wait();
    CLOG("server end");
}

TEST_CASE("AsyncServer", "[grpc]")
{
    std::thread serverThread([] {AsyncServer(); });
    serverThread.detach();

    while (true)
    {
        std::string line;
        std::getline(std::cin, line);
        if (line == "q")
        {
            break;
        }
    }
}

TEST_CASE("SyncClient", "[grpc]")
{
    std::thread clientThread([] { SyncClient(); }); // move this to another process and you you never get the cancellation
    clientThread.detach();

    while (true)
    {
        std::string line;
        std::getline(std::cin, line);
        if (line == "q")
        {
            break;
        }
    }
}

TEST_CASE("SyncClient_AsyncServer", "[grpc]")
{
    std::thread serverThread([] {AsyncServer(); });
    std::thread clientThread([] {SyncClient(); }); // move this to another process and you you never get the cancellation
    serverThread.detach();
    clientThread.detach();

   

    while (true)
    {
        std::string line;
        std::getline(std::cin, line);
        if (line == "q")
        {
            break;
        }
    }
}

TEST_CASE("AsyncCallbackServer", "[grpc]")
{
    std::jthread clientThread([] { AsyncCallbackServer(); }); // move this to another process and you you never get the cancellation
    clientThread.detach();
}

TEST_CASE("SyncServer", "[grpc]")
{
    std::thread serverThread([] {SyncServer(); });
    serverThread.detach();

    while (true)
    {
        std::string line;
        std::getline(std::cin, line);
        if (line == "q")
        {
            break;
        }
    }
}














TEST_CASE("server only", "[grpc]")
{

    const auto server_grpc_port = "50051";
    const auto server_host = std::string("0.0.0.0:") + server_grpc_port;

    rgo::VisualizationServer server;

    server.Start(server_host);


    std::atomic_bool active = true;
    std::jthread bk(
        [&]()
        {
            while (active)
            {
                //server.PollCompletions(100ms);
                server.RunCompletions();

            }
        }
    );


    std::string line;

    while (true)
    {
        std::getline(std::cin, line);
        if (line == "q")
        {
            break;
        }
    }

    //std::this_thread::sleep_for(20s);
    server.Stop();

    std::this_thread::sleep_for(2s);
    active = false;

    std::this_thread::sleep_for(2s);


}























void AsyncClient()
{

    const auto server_grpc_port = "50051";

    const auto client_host = std::string("localhost:") + server_grpc_port;

    eureka::RemoteLiveSlamClient client;



    client.ConnectAsync(client_host);

    std::atomic_bool active = true;
    std::jthread bk(
        [&]()
        {
            while (active)
            {

                client.PollCompletions(100ms);
            }
        }
    );

    client.StartReadingPoseGraphUpdates();
    std::this_thread::sleep_for(5s);
    client.StopReadingPoseGraphUpdates();
    std::this_thread::sleep_for(200s);
    active = false;

    std::this_thread::sleep_for(200s);
}



TEST_CASE("server only with client only", "[grpc]")
{
    const auto server_grpc_port = "50051";
    const auto server_host = std::string("localhost:") + server_grpc_port;

    rgo::VisualizationServer server;

    server.Start(server_host);
    std::jthread client(
        [&]()
        {
            AsyncClient();
        }
    );

    std::atomic_bool active = true;
    std::jthread bk(
        [&]()
        {
            while (active)
            {
                server.PollCompletions(100ms);

            }
        }
    );


    std::this_thread::sleep_for(20s);
    server.Stop();

    std::this_thread::sleep_for(2s);
    active = false;

    std::this_thread::sleep_for(2s);


}



TEST_CASE("client cancellation", "[grpc]")
{
    const auto server_grpc_port = "50051";
    const auto server_host = std::string("0.0.0.0:") + server_grpc_port;
    const auto client_host = std::string("localhost:") + server_grpc_port;
    rgo::VisualizationServer server;
    eureka::RemoteLiveSlamClient client;
    server.Start(server_host);
    client.ConnectAsync(client_host);

    std::atomic_bool active = true;
    std::jthread bk(
        [&]()
        {
            while (active)
            {
                server.PollCompletions(100ms);
                client.PollCompletions(100ms);
            }
        }
    );

    client.StartReadingPoseGraphUpdates();
    std::this_thread::sleep_for(10s);
    client.StopReadingPoseGraphUpdates();
    std::this_thread::sleep_for(1s);
    active = false;

    std::this_thread::sleep_for(1s);

}



TEST_CASE("AsyncClient", "[grpc]")
{
    AsyncClient();

}

