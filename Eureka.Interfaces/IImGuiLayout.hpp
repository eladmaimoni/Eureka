#pragma once

#ifndef EUREKA_NO_VTABLE
#ifdef _MSC_VER 
#define EUREKA_NO_VTABLE __declspec(novtable)
#else
#define EUREKA_NO_VTABLE
#endif
#endif
namespace eureka
{
    struct ImguiLayoutProps
    {
        bool has_ini_file = false;
    };

    class EUREKA_NO_VTABLE IImGuiLayout
    {
    public:
        virtual ~IImGuiLayout() = 0;
        virtual void OnActivated(const ImguiLayoutProps& props) = 0;
        virtual void OnDeactivated() = 0;
        virtual void UpdateLayout() = 0;
    };

}

