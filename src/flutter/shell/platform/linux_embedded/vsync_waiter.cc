// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/vsync_waiter.h"

#include <cassert>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

VsyncWaiter::VsyncWaiter() : event_counter_(0) {}

void VsyncWaiter::NotifyWaitForVsync(intptr_t baton) {
  std::lock_guard<std::mutex> lk(mutex_);
  baton_ = baton;
  event_counter_++;
}

void VsyncWaiter::NotifyVsync(FLUTTER_API_SYMBOL(FlutterEngine) engine,
                              FlutterEngineProcTable* embedder_api,
                              uint64_t frame_start_time_nanos,
                              uint64_t frame_target_time_nanos) {
  std::lock_guard<std::mutex> lk(mutex_);
  if (event_counter_ > 0 && baton_ != 0) {
    assert(event_counter_ == 1);
    event_counter_--;
    auto result = embedder_api->OnVsync(engine, baton_, frame_start_time_nanos,
                                        frame_target_time_nanos);
    if (result != kSuccess) {
      ELINUX_LOG(ERROR) << "FlutterEngineOnVsync failed: batton = " << baton_;
    }
  }
}

}  // namespace flutter
