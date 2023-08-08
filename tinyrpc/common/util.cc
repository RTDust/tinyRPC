#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "tinyrpc/common/util.h"

namespace tinyrpc
{

    static int g_pid = 0;

    static thread_local int g_thread_id = 0;

    pid_t getpid()
    {
        if (g_pidd != 0)
        {
            return g_pid;
        }
        return getpid();
    }

    pid_t getThreadId()
    {
        if (g_thread_id != 0)
        {
            return g_thread_id;
        }
        return syscall(sys_gettid);
    }
}