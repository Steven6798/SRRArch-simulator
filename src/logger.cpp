/**
 * @file logger.cpp
 * @brief Logging utility implementation
 *
 * Implements singleton logger with level filtering,
 * timestamp formatting, and thread-safe output.
 *
 * @author SRRArch Simulator Team
 * @version 0.1.0
 * @date 2026
 */

#include "logger.h"
#include <cstdio>

namespace srrarch {

void Logger::log(LogLevel level, const char *format, ...) {
  std::lock_guard<std::mutex> lock(m_mutex);

  // Add timestamp
  time_t now = time(nullptr);
  char timestamp[20];
  strftime(timestamp, sizeof(timestamp), "%H:%M:%S", localtime(&now));
  printf("[%s] ", timestamp);

  // Print level prefix
  const char *prefix;
  switch (level) {
  case LogLevel::ERROR:
    prefix = "[ERROR] ";
    break;
  case LogLevel::WARNING:
    prefix = "[WARN]  ";
    break;
  case LogLevel::INFO:
    prefix = "[INFO]  ";
    break;
  case LogLevel::DBG:
    prefix = "[DBG]   ";
    break;
  case LogLevel::TRACE:
    prefix = "[TRACE] ";
    break;
  default:
    prefix = "";
    break;
  }
  printf("%s", prefix);

  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);

  printf("\n");
  fflush(stdout);
}

} // namespace srrarch