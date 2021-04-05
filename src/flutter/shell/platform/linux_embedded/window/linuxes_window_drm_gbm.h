// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_LINUXES_WINDOW_DRM_GBM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_LINUXES_WINDOW_DRM_GBM_H_

#include "flutter/shell/platform/linux_embedded/surface/context_egl_drm_gbm.h"
#include "flutter/shell/platform/linux_embedded/surface/environment_egl.h"
#include "flutter/shell/platform/linux_embedded/window/linuxes_window_drm.h"
#include "flutter/shell/platform/linux_embedded/window/native_window_drm_gbm.h"

namespace flutter {

class LinuxesWindowDrmGbm
    : public LinuxesWindowDrm<NativeWindowDrmGbm, SurfaceGlDrmGbm> {
 public:
  using LinuxesWindowDrm::LinuxesWindowDrm;

  // |LinuxesWindowDrm|
  std::unique_ptr<SurfaceGlDrmGbm> CreateRenderSurface() override {
    return std::make_unique<SurfaceGlDrmGbm>(std::make_unique<ContextEglDrmGbm>(
        std::make_unique<EnvironmentEgl<gbm_device>>(
            native_window_->BufferDevice())));
  }
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_LINUXES_WINDOW_DRM_GBM_H_