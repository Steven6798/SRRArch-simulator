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

Logger &Logger::instance() {
  static Logger instance;
  return instance;
}

void Logger::printPrefix(LogLevel level) {
  if (m_level == LogLevel::NONE)
    return;

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
}

} // namespace srrarch