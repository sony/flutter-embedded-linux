// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "logger.h"

namespace flutter {

namespace {

#ifdef FLUTTER_RELEASE
constexpr int kFilterLogLevel = LINUXES_LOG_WARNING;
#else
constexpr int kFilterLogLevel = LINUXES_LOG_TRACE;
#endif

const char* const kLogLevelNames[LINUXES_LOG_NUM] = {"TRACE", "INFO", "WARNING",
                                                     "ERROR", "FATAL"};

const char* GetLogLevelName(int level) {
  if (LINUXES_LOG_TRACE <= level && level < LINUXES_LOG_NUM)
    return kLogLevelNames[level];
  return "UNKNOWN";
}

}  // namespace

Logger::Logger(int level, const char* file, int line) : level_(level) {
  if (level_ < kFilterLogLevel) {
    return;
  }
  stream_ << "[" << GetLogLevelName(level_) << "]";
  stream_ << "[" << file << "(" << line << ")] ";
}

Logger::~Logger() {
  if (level_ < kFilterLogLevel) {
    return;
  }

  stream_ << std::endl;
  std::cerr << stream_.str();
  std::cerr.flush();
  if (level_ >= LINUXES_LOG_FATAL) {
    abort();
  }
}

}  // namespace flutter