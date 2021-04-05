// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_DRM_GBM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_DRM_GBM_H_

#include <gbm.h>

#include "flutter/shell/platform/linux_embedded/surface/context_egl_drm_gbm.h"
#include "flutter/shell/platform/linux_embedded/surface/linuxes_surface_gl_drm.h"
#include "flutter/shell/platform/linux_embedded/window/native_window_drm_gbm.h"

namespace flutter {

class SurfaceGlDrmGbm final
    : public SurfaceGlDrm<gbm_surface, ContextEglDrmGbm> {
 public:
  using SurfaceGlDrm::SurfaceGlDrm;

  // |SurfaceGlDelegate|
  bool GLContextPresent(uint32_t fbo_id) const override {
    if (!onscreen_surface_->SwapBuffers()) {
      return false;
    }
    static_cast<NativeWindowDrmGbm*>(native_window_)->SwapBuffer();
    return true;
  }
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_DRM_GBM_H_