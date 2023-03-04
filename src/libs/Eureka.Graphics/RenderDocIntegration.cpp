
#include "RenderDocIntegration.hpp"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h> // GetModuleHandleA
#endif
#include <iostream>
namespace eureka
{

    RenderDocIntegration::RenderDocIntegration()
    {
#ifdef _WIN32
        auto renderDocModule = GetModuleHandleA("renderdoc.dll");
        if (renderDocModule)
        {
            pRENDERDOC_GetAPI RENDERDOC_GetAPI = reinterpret_cast<pRENDERDOC_GetAPI>(GetProcAddress(renderDocModule, "RENDERDOC_GetAPI"));
            [[maybe_unused]] int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_5_0, reinterpret_cast<void**>(&_api));
            assert(ret == 1);

            //_api->SetCaptureOptionU32(RENDERDOC_CaptureOption::)

             std::cout << "RenderDoc API integration is active\n";
        }
#endif
    }
    RenderDocIntegration::~RenderDocIntegration()
    {
    
    }

    bool RenderDocIntegration::IsAttached() const
    {
        return _api != nullptr;
    }

    void RenderDocIntegration::StartCapture()
    {
        if (_api)
        {
            _api->StartFrameCapture(NULL, NULL);
        }
    }

    void RenderDocIntegration::StartCapture(VkDevice device)
    {
        if (_api)
        {
            _api->StartFrameCapture(device, NULL);
        }
    }

    void RenderDocIntegration::EndCapture(VkDevice device)
    {
        if (_api)
        {
            _api->EndFrameCapture(device, NULL);
        }
    }

    void RenderDocIntegration::EndCapture()
    {
        if (_api)
        {
            _api->EndFrameCapture(NULL, NULL);
        }
    }
}



