// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_LINUXES_WINDOW_DRM_EGLSTREAM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_LINUXES_WINDOW_DRM_EGLSTREAM_H_

#include "flutter/shell/platform/linux_embedded/surface/context_egl_drm_eglstream.h"
#include "flutter/shell/platform/linux_embedded/surface/environment_egl_drm_eglstream.h"
#include "flutter/shell/platform/linux_embedded/window/linuxes_window_drm.h"
#include "flutter/shell/platform/linux_embedded/window/native_window_drm_eglstream.h"

namespace flutter {

class LinuxesWindowDrmEglstream
    : public LinuxesWindowDrm<NativeWindowDrmEglstream, SurfaceGlDrmEglstream> {
 public:
  using LinuxesWindowDrm::LinuxesWindowDrm;

  // |LinuxesWindowDrm|
  std::unique_ptr<SurfaceGlDrmEglstream> CreateRenderSurface() override {
    return std::make_unique<SurfaceGlDrmEglstream>(
        std::make_unique<ContextEglDrmEglstream>(
            std::make_unique<EnvironmentEglDrmEglstream>(
                native_window_->DrmDevice())));
  }
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_LINUXES_WINDOW_DRM_EGLSTREAM_H_