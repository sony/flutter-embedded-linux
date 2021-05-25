// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_VSYNC_WAITER_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_VSYNC_WAITER_H_

#include <memory>
#include <mutex>

#include "flutter/shell/platform/embedder/embedder.h"

namespace flutter {

class VsyncWaiter {
 public:
  VsyncWaiter(FLUTTER_API_SYMBOL(FlutterEngine) engine,
              FlutterEngineProcTable* table);
  ~VsyncWaiter() = default;

  void NotifyWaitForVsync(intptr_t baton);

  void NotifyVsync(uint64_t frame_start_time_nanos,
                   uint64_t frame_target_time_nanos);

 private:
  FLUTTER_API_SYMBOL(FlutterEngine) engine_;
  FlutterEngineProcTable* embedder_api_;

  intptr_t baton_;
  uint32_t event_counter_;
  std::mutex mutex_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_VSYNC_WAITER_H_
