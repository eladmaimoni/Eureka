#include "thread_name.hpp"
#include <cstdint>

namespace eureka
{
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

    const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
    typedef struct tagTHREADNAME_INFO
    {
        DWORD dwType; // Must be 0x1000.
        LPCSTR szName; // Pointer to name (in user addr space).
        DWORD dwThreadID; // Thread ID (-1=caller thread).
        DWORD dwFlags; // Reserved for future use, must be zero.
    } THREADNAME_INFO;
#pragma pack(pop)


    void set_thread_name_by_id(uint32_t dwThreadID, const char* threadName)
    {
        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = threadName;
        info.dwThreadID = dwThreadID;
        info.dwFlags = 0;

        __try
        {
            RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }
    void set_current_thread_name(const char* threadName)
    {
        set_thread_name_by_id(GetCurrentThreadId(), threadName);
    }


    void set_thread_name(void* thread_handle, const char* threadName)
    {
        DWORD threadId = GetThreadId(static_cast<HANDLE>(thread_handle));
        set_thread_name_by_id(threadId, threadName);
    }

#elif defined(__linux__)
#include <sys/prctl.h>
    void set_current_thread_name(const char* threadName)
    {
        prctl(PR_SET_NAME, threadName, 0, 0, 0);
    }

#else
    void set_thread_name(void* thread_handle, const char* threadName)
    {
        pthread_setname_np(thread_handle, threadName);
    }
#endif
}

