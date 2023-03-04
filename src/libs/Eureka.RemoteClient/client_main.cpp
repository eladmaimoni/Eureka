//#include <compiler.hpp>
#include "RemoteLiveSlamControlCenterHandler.hpp"
#include <logging.hpp>
//EUREKA_MSVC_WARNING_PUSH
//EUREKA_MSVC_WARNING_DISABLE(4127)
//EUREKA_MSVC_WARNING_POP

//#include "helper.hpp"

#include <agrpc/asio_grpc.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>


int main2()
{
    try
    {
        const auto server_grpc_port = "50051";
        const auto host = std::string("localhost:") + server_grpc_port;

        auto completionQueue = std::make_shared<eureka::ClientCompletionQueuePollingExecutor>();

        eureka::RemoteLiveSlamControlCenterHandler client;
        client.ConnectAsync(host);

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
                client.ConnectAsync(host);
            }
            else if (line == "stop")
            {
                client.Stop();
            }
            else if (line == "s")
            {
                client.SendRequest();
            }
            else if (line == "p")
            {
                client.StartReadingPoseGraphUpdates();
            }
        }

        CLOG("client quitting ...");
    }
    catch (const std::exception& err)
    {
        CLOG("client error {}", err.what());
    }

    CLOG("client main thread exited");

    return 0;
}

