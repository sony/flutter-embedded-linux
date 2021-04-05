// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_DRM_EGLSTREAM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_DRM_EGLSTREAM_H_

#include "flutter/shell/platform/linux_embedded/surface/context_egl_drm_eglstream.h"
#include "flutter/shell/platform/linux_embedded/surface/linuxes_surface_gl_drm.h"

namespace flutter {

class SurfaceGlDrmEglstream final
    : public SurfaceGlDrm<uint32_t, ContextEglDrmEglstream> {
 public:
  using SurfaceGlDrm::SurfaceGlDrm;

  // |SurfaceGlDelegate|
  bool GLContextPresent(uint32_t fbo_id) const override {
    return onscreen_surface_->SwapBuffers();
  }
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_DRM_EGLSTREAM_H_