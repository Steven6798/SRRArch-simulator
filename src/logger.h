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

#include <cstdio>
#include <mutex>
#include <string>
#include <time.h>

namespace srrarch {

enum class LogLevel {
  NONE = 0,    // No logging at all (for long simulations)
  ERROR = 1,   // Only errors
  WARNING = 2, // Errors + warnings
  INFO = 3,    // Normal operational info
  DBG = 4,     // Detailed debugging info
  TRACE = 5    // Extremely verbose (instruction trace)
};

class Logger {
public:
  static Logger &instance();

  void setLevel(LogLevel level) { m_level = level; }
  LogLevel getLevel() const { return m_level; }

  template <typename... Args>
  void log(LogLevel level, const char *format, Args... args) {
    if (static_cast<int>(level) <= static_cast<int>(m_level)) {
      std::lock_guard<std::mutex> lock(m_mutex);

      // Add timestamp for non-NONE levels
      if (m_level != LogLevel::NONE) {
        time_t now = time(nullptr);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%H:%M:%S", localtime(&now));
        printf("[%s] ", timestamp);
      }

      printPrefix(level);

// Suppress format string warnings
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wformat-security"
      printf(format, args...);
#pragma GCC diagnostic pop

      printf("\n");
      fflush(stdout);
    }
  }

  // Quick check to avoid expensive operations when logging is disabled
  bool shouldLog(LogLevel level) const {
    return static_cast<int>(level) <= static_cast<int>(m_level);
  }

private:
  Logger() = default;
  void printPrefix(LogLevel level);

  LogLevel m_level = LogLevel::INFO;
  std::mutex m_mutex;
};

} // namespace srrarch

// Convenience macros
#define LOG_NONE(...) ((void)0)
#define LOG_ERROR(...)                                                         \
  do {                                                                         \
    if (srrarch::Logger::instance().shouldLog(srrarch::LogLevel::ERROR))       \
      srrarch::Logger::instance().log(srrarch::LogLevel::ERROR, __VA_ARGS__);  \
  } while (0)

#define LOG_WARN(...)                                                          \
  do {                                                                         \
    if (srrarch::Logger::instance().shouldLog(srrarch::LogLevel::WARNING))     \
      srrarch::Logger::instance().log(srrarch::LogLevel::WARNING,              \
                                      __VA_ARGS__);                            \
  } while (0)

#define LOG_INFO(...)                                                          \
  do {                                                                         \
    if (srrarch::Logger::instance().shouldLog(srrarch::LogLevel::INFO))        \
      srrarch::Logger::instance().log(srrarch::LogLevel::INFO, __VA_ARGS__);   \
  } while (0)

#define LOG_DBG(...)                                                           \
  do {                                                                         \
    if (srrarch::Logger::instance().shouldLog(srrarch::LogLevel::DBG))         \
      srrarch::Logger::instance().log(srrarch::LogLevel::DBG, __VA_ARGS__);    \
  } while (0)

#define LOG_TRACE(...)                                                         \
  do {                                                                         \
    if (srrarch::Logger::instance().shouldLog(srrarch::LogLevel::TRACE))       \
      srrarch::Logger::instance().log(srrarch::LogLevel::TRACE, __VA_ARGS__);  \
  } while (0)

// Conditional logging macros
#define LOG_IF(level, cond, ...)                                               \
  do {                                                                         \
    if ((cond))                                                                \
      LOG_##level(__VA_ARGS__);                                                \
  } while (0)

#endif // SRRARCH_LOGGER_H