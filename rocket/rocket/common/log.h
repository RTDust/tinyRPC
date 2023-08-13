#ifndef ROCKET_COMMON_LOG_H
#define ROCKET_COMMON_LOG_H

#include <string>
#include <vector>
#include <memory>
#include <queue>

namespace rocket
{

    template <typename... Args>
    std::string formatString(const char *str, Args &&...args)
    {
        int size = snprintf(nullptr, 0, str, args...);

        std::string result;
        if (size > 0)
        {
            result.resize(size);
            snprintf(&result[0], size + 1, str, args...);
        }
        return result;
    }

#define DEBUGLOG(str, ...)                                                                                                    \
    std::string msg = (new rocket::LogEvent(rocket::LogLevel::Debug))->toString() + rocket::formatString(str, ##__VA_ARGS__); \
    msg += "\n";                                                                                                              \
    rocket::Logger::GetGlobalLogger()->pushLog(msg);                                                                          \
    rocket::Logger::GetGlobalLogger()->log();

    enum LogLevel // 日志级别枚举
    {
        Debug = 1,
        Info = 2,
        Error = 3
    };

    class Logger
    {
    public:
        typedef std::shared_ptr<Logger> s_ptr;

        void pushLog(const std::string &msg);

        void log();

    public:
        static Logger *GetGlobalLogger();

    private:
        LogLevel m_set_level;

        std::queue<std::string> m_buffer;
    };

    std::string LogLevelToString(LogLevel level);

    class LogEvent
    {
    public:
        LogEvent(LogLevel level) : m_level(level) {}

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
        int32_t m_file_line;
        int32_t m_pid;
        int32_t m_thread_id;

        LogLevel m_level;
    };

}

#endif