#include <logging.hpp>
#include <LiveSlamServer.hpp>
#include <VisualizationService.hpp>
#include <GrpcContext.hpp>


int main()
{
    eureka::InitializeDefaultLogger();
    try
    {
        const auto server_grpc_port = "50051";
        const auto server_endpoint = std::string("0.0.0.0:") + server_grpc_port;
        auto service = std::make_shared<rgoproto::LiveSlamUIService::AsyncService>();
        auto liveSlamServer = std::make_shared<eureka::rpc::LiveSlamServer>(std::vector<std::shared_ptr<grpc::Service>>{ service });
        auto grpcContext = liveSlamServer->GetContext();
        auto visService = std::make_shared<eureka::rpc::VisualizationService>(service, grpcContext);
        liveSlamServer->Start(server_endpoint);
        //visService->Start();
        std::atomic_bool active = true;

        std::thread bk(
            [&]()
            {
                try
                {
                    while (active)
                    {
                        grpcContext->RunFor(1ms);
                    }
                }
                catch (const std::exception& err)
                {
                    EUREKA_LOG_ERROR("error {}", err.what());
                }

            }
        );

        while (true)
        {
            std::string line;
            std::getline(std::cin, line);

            if (line == "q")
            {
                EUREKA_LOG_INFO("quitting...");
                break;
            }
            else if (line == "service start")
            {
                EUREKA_LOG_INFO("starting service");
                visService->Start();
            }
            else if (line == "service stop")
            {
                EUREKA_LOG_INFO("stopping service");
                visService->Stop();
            }


        }

 

    }
    catch (const std::exception& err)
    {
        EUREKA_LOG_ERROR("error {}", err.what());
    }
}