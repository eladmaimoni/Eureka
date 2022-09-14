#include "profiling_categories.hpp"
#ifdef PERFETTO_TRACING
// Reserves internal static storage for our tracing categories.
PERFETTO_TRACK_EVENT_STATIC_STORAGE();
#endif