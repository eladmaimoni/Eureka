#pragma once

#include "renderdoc_app.h"

namespace eureka
{

    class RenderDocIntegration
    {
    public:
        RenderDocIntegration();
        ~RenderDocIntegration();
        EUREKA_NO_COPY_NO_MOVE(RenderDocIntegration);
        bool IsAttached() const;
        void StartCapture();
        void EndCapture();
        void StartCapture(VkDevice device);
        void EndCapture(VkDevice device);
    private:
        RENDERDOC_API_1_5_0* _api{ nullptr };
    };

    inline RenderDocIntegration* RenderDockIntegrationInstance = nullptr;

}