#ifndef TINYRPC_COMMON_LOG_H
#define TINYRPC_COMMON_LOG_H

#include <string>

namespace tinyrpc
{
    // 采用C++模板以及可变参数长度
    template <typename... Args>
    std::string formatString(const char *str, Args &&...args)
    {

        int size = snprintf(nullptr, 0, str, args...); // 通过snprintf函数获取整个格式化后的长度

        std::string result;
        if (size > 0)
        {
            result.resize(size);
            snprintf(&result[0], size + 1, str, args...);
        }

        return result;
    }

    enum LogLevel
    {
        Debug = 1,
        Info = 2,
        Error = 3
    };

    std::string LogLevelToString(LogLevel level);

    class LogEvent
    {
    public:
        std::string getFileName() const
        {
            return m_file_name;
        }

        LogLevel getLogLevel() const
        {
            return m_level;
        }

        std::string toString();

    private:
        std::string m_file_name; // 文件名
        int32_t m_file_line;     // 行号
        int32_t m_pid;           // 进程号
        int32_t m_thread_id;     // 线程号

        LogLevel m_level; // 日志级别
    };
}

#endif