#ifndef ROCKET_COMMON_RUN_TIME_H
#define ROCKET_COMMON_RUN_TIME_H

#include <string>

namespace rocket
{

    class RpcInterface;
    /// @brief 用于生成msg_id
    class RunTime
    {
    public:
        RpcInterface *getRpcInterface();

    public:
        static RunTime *GetRunTime();

    public:
        std::string m_msgid;//使用字符串类型来保存msg_id
        std::string m_method_name;
        RpcInterface *m_rpc_interface{NULL};
    };

}

#endif