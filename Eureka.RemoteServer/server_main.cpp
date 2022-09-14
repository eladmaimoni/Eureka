#include <compiler.hpp>
#include "RemoteUIServer.hpp"

#include <logging.hpp>

using namespace std::literals::chrono_literals;

int main()
{
    try
    {
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

