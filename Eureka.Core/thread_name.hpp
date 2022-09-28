

namespace eureka::os
{
    void set_current_thread_name(const char* threadName);
    void set_thread_name(void* thread_handle, const char* threadName);
}
