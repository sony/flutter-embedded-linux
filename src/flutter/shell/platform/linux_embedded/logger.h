// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_LOGGER_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_LOGGER_H_

#include <string.h>

#include <iostream>
#include <sstream>

namespace flutter {

constexpr int LINUXES_LOG_TRACE = 0;
constexpr int LINUXES_LOG_INFO = 1;
constexpr int LINUXES_LOG_WARNING = 2;
constexpr int LINUXES_LOG_ERROR = 3;
constexpr int LINUXES_LOG_FATAL = 4;
constexpr int LINUXES_LOG_NUM = 5;

#define __LOG_FILE_NAME__ \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LINUXES_LOG(level) \
  Logger(LINUXES_LOG_##level, __LOG_FILE_NAME__, __LINE__).stream()

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