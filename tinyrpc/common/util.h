#ifndef TINYRPC_COMMON_LOG_H
#define TINYRPC_COMMON_LOG_H

#include "tinyrpc/common/util.h"
#include <sys/time.h>

namespace tinyrpc
{

    pid_t getPid();

    pid_t getThreadId();

}

#endif