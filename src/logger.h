/**
 * @file logger.h
 * @brief Thread-safe logging utility
 *
 * Provides logging with multiple levels (DBG, INFO, WARNING, ERROR),
 * timestamps, and printf-style formatting. Thread-safe with mutex
 * protection.
 *
 * @author SRRArch Simulator Team
 * @version 0.1.0
 * @date 2026
 */

#ifndef SRRARCH_LOGGER_H
#define SRRARCH_LOGGER_H

#include <cstdarg>
#include <cstdio>
#include <mutex>
#include <string>
#include <time.h>

namespace srrarch {

// Log level enum (must match compile-time constants)
enum class LogLevel {
  NONE = 0,
  ERROR = 1,
  WARNING = 2,
  INFO = 3,
  DBG = 4,
  TRACE = 5
};

// Compile-time log level constants
#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4
#define LOG_LEVEL_TRACE 5

// Default log levels based on build type
#ifndef CURRENT_LOG_LEVEL
#ifdef NDEBUG
#define CURRENT_LOG_LEVEL LOG_LEVEL_NONE // Release: no logs
#else
#define CURRENT_LOG_LEVEL LOG_LEVEL_INFO // Debug: info level
#endif
#endif

class Logger {
public:
  static Logger &instance() {
    static Logger instance;
    return instance;
  }

  void setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_level = level;
  }

  LogLevel getLevel() const { return m_level; }

  void log(LogLevel level, const char *format, ...);

private:
  Logger() = default;

  LogLevel m_level = LogLevel::INFO;
  std::mutex m_mutex;
};

} // namespace srrarch

// Conditional logging macros - completely eliminated at compile time when
// disabled
#if CURRENT_LOG_LEVEL >= LOG_LEVEL_ERROR
#define LOG_ERROR(...)                                                         \
  do {                                                                         \
    if (static_cast<int>(srrarch::Logger::instance().getLevel()) >=            \
        static_cast<int>(srrarch::LogLevel::ERROR)) {                          \
      srrarch::Logger::instance().log(srrarch::LogLevel::ERROR, __VA_ARGS__);  \
    }                                                                          \
  } while (0)
#else
#define LOG_ERROR(...) ((void)0)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_WARNING
#define LOG_WARN(...)                                                          \
  do {                                                                         \
    if (static_cast<int>(srrarch::Logger::instance().getLevel()) >=            \
        static_cast<int>(srrarch::LogLevel::WARNING)) {                        \
      srrarch::Logger::instance().log(srrarch::LogLevel::WARNING,              \
                                      __VA_ARGS__);                            \
    }                                                                          \
  } while (0)
#else
#define LOG_WARN(...) ((void)0)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_INFO
#define LOG_INFO(...)                                                          \
  do {                                                                         \
    if (static_cast<int>(srrarch::Logger::instance().getLevel()) >=            \
        static_cast<int>(srrarch::LogLevel::INFO)) {                           \
      srrarch::Logger::instance().log(srrarch::LogLevel::INFO, __VA_ARGS__);   \
    }                                                                          \
  } while (0)
#else
#define LOG_INFO(...) ((void)0)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_DEBUG
#define LOG_DBG(...)                                                           \
  do {                                                                         \
    if (static_cast<int>(srrarch::Logger::instance().getLevel()) >=            \
        static_cast<int>(srrarch::LogLevel::DEBUG)) {                          \
      srrarch::Logger::instance().log(srrarch::LogLevel::DEBUG, __VA_ARGS__);  \
    }                                                                          \
  } while (0)
#else
#define LOG_DBG(...) ((void)0)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_TRACE
#define LOG_TRACE(...)                                                         \
  do {                                                                         \
    if (static_cast<int>(srrarch::Logger::instance().getLevel()) >=            \
        static_cast<int>(srrarch::LogLevel::TRACE)) {                          \
      srrarch::Logger::instance().log(srrarch::LogLevel::TRACE, __VA_ARGS__);  \
    }                                                                          \
  } while (0)
#else
#define LOG_TRACE(...) ((void)0)
#endif

#endif // SRRARCH_LOGGER_H