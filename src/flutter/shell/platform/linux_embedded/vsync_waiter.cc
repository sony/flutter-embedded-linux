// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/vsync_waiter.h"

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

VsyncWaiter::VsyncWaiter() : event_counter_(0) {}

void VsyncWaiter::NotifyWaitForVsync(intptr_t baton) {
  std::lock_guard<std::mutex> lk(mutex_);
  baton_ = baton;
  event_counter_++;
  LINUXES_LOG(TRACE) << "NotifyWaitForVsync: batton = " << baton
                     << ", counter = " << event_counter_;
}

void VsyncWaiter::NotifyVsync(FLUTTER_API_SYMBOL(FlutterEngine) engine,
                              FlutterEngineProcTable* embedder_api,
                              uint64_t frame_start_time_nanos,
                              uint64_t frame_target_time_nanos) {
  std::lock_guard<std::mutex> lk(mutex_);
  if (event_counter_ > 0 && baton_ != 0) {
    LINUXES_LOG(TRACE) << "NotifyVsync: counter = " << event_counter_
                       << ", start = " << frame_start_time_nanos
                       << ", end = " << frame_target_time_nanos;

    event_counter_--;
    auto result = embedder_api->OnVsync(engine, baton_, frame_start_time_nanos,
                                        frame_target_time_nanos);
    if (result != kSuccess) {
      LINUXES_LOG(TRACE) << "FlutterEngineOnVsync failed: batton = " << baton_;
    }
  }
}

}  // namespace flutter
