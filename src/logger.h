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

enum class LogLevel { DBG, INFO, WARNING, ERROR };

class Logger {
public:
  static Logger &instance();

  void setLevel(LogLevel level);

  template <typename... Args>
  void log(LogLevel level, const char *format, Args... args) {
    if (level >= m_level) {
      std::lock_guard<std::mutex> lock(m_mutex);

      // Add timestamp
      time_t now = time(nullptr);
      char timestamp[20];
      strftime(timestamp, sizeof(timestamp), "%H:%M:%S", localtime(&now));
      printf("[%s] ", timestamp);

      printPrefix(level);

// Suppress format string warnings for this specific call
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wformat-security"
      printf(format, args...);
#pragma GCC diagnostic pop

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

} // namespace srrarch

#define LOG_DBG(...)                                                           \
  srrarch::Logger::instance().log(srrarch::LogLevel::DBG, __VA_ARGS__)
#define LOG_INFO(...)                                                          \
  srrarch::Logger::instance().log(srrarch::LogLevel::INFO, __VA_ARGS__)
#define LOG_WARN(...)                                                          \
  srrarch::Logger::instance().log(srrarch::LogLevel::WARNING, __VA_ARGS__)
#define LOG_ERROR(...)                                                         \
  srrarch::Logger::instance().log(srrarch::LogLevel::ERROR, __VA_ARGS__)

#endif // SRRARCH_LOGGER_H