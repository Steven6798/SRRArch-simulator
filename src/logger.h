#ifndef SRRARCH_LOGGER_H
#define SRRARCH_LOGGER_H

#include <string>
#include <cstdio>
#include <mutex>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static Logger& instance();

    void setLevel(LogLevel level);

    template<typename... Args>
    void log(LogLevel level, const char* format, Args... args) {
        if (level >= m_level) {
            std::lock_guard<std::mutex> lock(m_mutex);
            printPrefix(level);
            printf(format, args...);
            printf("\n");
            fflush(stdout);
        }
    }

private:
    Logger() = default;
    void printPrefix(LogLevel level);

    LogLevel m_level = LogLevel::INFO;
    std::mutex m_mutex;
};

#define LOG_DEBUG(...) Logger::instance().log(LogLevel::DEBUG, __VA_ARGS__)
#define LOG_INFO(...)  Logger::instance().log(LogLevel::INFO, __VA_ARGS__)
#define LOG_WARN(...)  Logger::instance().log(LogLevel::WARNING, __VA_ARGS__)
#define LOG_ERROR(...) Logger::instance().log(LogLevel::ERROR, __VA_ARGS__)

#endif