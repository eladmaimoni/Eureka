#include "Profiling.hpp"

#if EUREKA_HAS_NVTOOLSEXT
#include <nvtx3/nvToolsExt.h>


namespace Profiling
{
    void InitProfilingCategories()
    {
        
        nvtxNameCategoryA(PROFILING_CATEGORY_LOAD, "load");
        nvtxNameCategoryA(PROFILING_CATEGORY_INIT, "init");
        nvtxNameCategoryA(PROFILING_CATEGORY_RENDERING, "rendering");
    }

    void PushRange(const char* rangeName, Color color, uint32_t category)
    {
        nvtxEventAttributes_t eventAttrib{};
        eventAttrib.version = NVTX_VERSION;
        eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
        eventAttrib.colorType = NVTX_COLOR_ARGB;
        eventAttrib.color = static_cast<uint32_t>(color);
        eventAttrib.messageType = NVTX_MESSAGE_TYPE_ASCII;
        eventAttrib.message.ascii = rangeName;
        eventAttrib.category = category;
        nvtxRangePushEx(&eventAttrib);
    }

    uint64_t StartUnthreadedRange(const char* rangeName, Color color, uint32_t category)
    {
        nvtxEventAttributes_t eventAttrib{};
        eventAttrib.version = NVTX_VERSION;
        eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
        eventAttrib.colorType = NVTX_COLOR_ARGB;
        eventAttrib.color = static_cast<uint32_t>(color);
        eventAttrib.messageType = NVTX_MESSAGE_TYPE_ASCII;
        eventAttrib.message.ascii = rangeName;
        eventAttrib.category = category;
        return nvtxRangeStartEx(&eventAttrib);
    }

    void EndUnthreadedRange(uint64_t rangeId)
    {
        nvtxRangeEnd(rangeId);
    }

    void PushRange(const char* rangeName)
    {
        nvtxRangePushA(rangeName);
    }

    void PopRange()
    {
        nvtxRangePop();
    }
    void SetProfilingMark(const char* markName)
    {
        nvtxMarkA(markName);
    }
    void SetProfilingMark(const char* markName, Color color, uint32_t category)
    {
        nvtxEventAttributes_t eventAttrib{};
        eventAttrib.version = NVTX_VERSION;
        eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
        eventAttrib.colorType = NVTX_COLOR_ARGB;
        eventAttrib.color = static_cast<uint32_t>(color);
        eventAttrib.messageType = NVTX_MESSAGE_TYPE_ASCII;
        eventAttrib.message.ascii = markName;
        eventAttrib.category = category;
        nvtxMarkEx(&eventAttrib);
    }
    void Profiling::NameCurrentThreadW(uint32_t id, const wchar_t* name)
    {
        nvtxNameOsThreadW(id, name);
    }
    ProfileScope::ProfileScope(const char* rangeName, Color color, uint32_t category)
    {
        PushRange(rangeName, color, category);
    }

    ProfileScope::ProfileScope(const char* rangeName)
    {
        PushRange(rangeName);
    }

    ProfileScope::~ProfileScope()
    {
        PopRange();
    }

    ProfileUnthreadedScope::ProfileUnthreadedScope(const char* rangeName, Color color, uint32_t category /*= 0*/)
    {
        _id = StartUnthreadedRange(rangeName, color, category);
    }
    ProfileUnthreadedScope::~ProfileUnthreadedScope()
    {
        EndUnthreadedRange(_id);
    }
}

#endif
