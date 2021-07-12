// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_LOGGER_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_LOGGER_H_

#include <string.h>

#include <iostream>
#include <sstream>

namespace flutter {

constexpr int ELINUX_LOG_TRACE = 0;
constexpr int ELINUX_LOG_DEBUG = 1;
constexpr int ELINUX_LOG_INFO = 2;
constexpr int ELINUX_LOG_WARNING = 3;
constexpr int ELINUX_LOG_ERROR = 4;
constexpr int ELINUX_LOG_FATAL = 5;
constexpr int ELINUX_LOG_NUM = 6;

#if defined(NDEBUG)
#define ELINUX_LOG(level) Logger(-1, "", 0).stream()
#else
#define __LOG_FILE_NAME__ \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define ELINUX_LOG(level) \
  Logger(ELINUX_LOG_##level, __LOG_FILE_NAME__, __LINE__).stream()
#endif

class Logger {
 public:
  Logger(int level, const char* file, int line);
  ~Logger();

  std::ostream& stream() { return stream_; }

 private:
  const int level_;
  std::ostringstream stream_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_LOGGER_H_