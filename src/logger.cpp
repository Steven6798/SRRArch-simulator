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

void Logger::setLevel(LogLevel level) { m_level = level; }

void Logger::printPrefix(LogLevel level) {
  const char *prefix;
  switch (level) {
  case LogLevel::DBG:
    prefix = "[DBG] ";
    break;
  case LogLevel::INFO:
    prefix = "[INFO]  ";
    break;
  case LogLevel::WARNING:
    prefix = "[WARN]  ";
    break;
  case LogLevel::ERROR:
    prefix = "[ERROR] ";
    break;
  }
  printf("%s", prefix);
}

} // namespace srrarch