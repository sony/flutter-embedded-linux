// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "logger.h"

#include <cstring>
#include <unordered_map>

namespace flutter {

namespace {

constexpr char kFlutterLogLevelsEnvironmentKey[] = "FLUTTER_LOG_LEVELS";
constexpr char kFlutterLogLevelTrace[] = "TRACE";
constexpr char kFlutterLogLevelDebug[] = "DEBUG";
constexpr char kFlutterLogLevelInfo[] = "INFO";
constexpr char kFlutterLogLevelWarning[] = "WARNING";
constexpr char kFlutterLogLevelError[] = "ERROR";
constexpr char kFlutterLogLevelFatal[] = "FATAL";
constexpr char kFlutterLogLevelUnknown[] = "UNKNOWN";

const char* const kLogLevelNames[ELINUX_LOG_NUM] = {
    kFlutterLogLevelTrace,   kFlutterLogLevelDebug, kFlutterLogLevelInfo,
    kFlutterLogLevelWarning, kFlutterLogLevelError, kFlutterLogLevelFatal};

const std::unordered_map<std::string, int> gLogLevelsMap{
    {kFlutterLogLevelTrace, ELINUX_LOG_TRACE},
    {kFlutterLogLevelDebug, ELINUX_LOG_DEBUG},
    {kFlutterLogLevelInfo, ELINUX_LOG_INFO},
    {kFlutterLogLevelWarning, ELINUX_LOG_WARNING},
    {kFlutterLogLevelError, ELINUX_LOG_ERROR},
    {kFlutterLogLevelFatal, ELINUX_LOG_FATAL},
};

int gFilterLogLevel = -1;

int GetCurrentLogLevel() {
  if (gFilterLogLevel == -1) {
    auto env_log_level = std::getenv(kFlutterLogLevelsEnvironmentKey);
    if (!env_log_level || (env_log_level[0] == '\0')) {
      gFilterLogLevel = ELINUX_LOG_WARNING;
    } else {
      if (gLogLevelsMap.find(env_log_level) != gLogLevelsMap.end()) {
        gFilterLogLevel = gLogLevelsMap.at(env_log_level);
      } else {
        gFilterLogLevel = ELINUX_LOG_WARNING;
      }
    }
  }
  return gFilterLogLevel;
}

const char* GetLogLevelName(int level) {
  if (ELINUX_LOG_TRACE <= level && level < ELINUX_LOG_NUM)
    return kLogLevelNames[level];
  return kFlutterLogLevelUnknown;
}

}  // namespace

Logger::Logger(int level, const char* file, int line) : level_(level) {
  if (level_ < GetCurrentLogLevel()) {
    return;
  }

  stream_ << "[" << GetLogLevelName(level_) << "]";
  stream_ << "[" << file << "(" << line << ")] ";
}

Logger::~Logger() {
  if (level_ < GetCurrentLogLevel()) {
    return;
  }

  stream_ << std::endl;
  std::cerr << stream_.str();
  std::cerr.flush();
  if (level_ >= ELINUX_LOG_FATAL) {
    abort();
  }
}

}  // namespace flutter