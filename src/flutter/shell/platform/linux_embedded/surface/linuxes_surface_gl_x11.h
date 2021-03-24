// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_X11_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_X11_H_

#include <xcb/xcb.h>

#include <memory>

#include "flutter/shell/platform/linux_embedded/surface/context_egl_x11.h"
#include "flutter/shell/platform/linux_embedded/surface/linuxes_egl_surface.h"
#include "flutter/shell/platform/linux_embedded/surface/linuxes_surface.h"
#include "flutter/shell/platform/linux_embedded/surface/linuxes_surface_gl_delegate.h"

namespace flutter {

class SurfaceGlX11 final : public Surface<xcb_window_t>,
                           public SurfaceGlDelegate {
 public:
  SurfaceGlX11(std::unique_ptr<ContextEglX11> context);
  ~SurfaceGlX11() = default;

  // |Surface|
  bool IsValid() const override;

  // |Surface|
  bool SetNativeWindow(NativeWindow<xcb_window_t>* window) override;

  // |Surface|
  bool OnScreenSurfaceResize(const size_t width,
                             const size_t height) const override;

  // |Surface|
  void DestroyOnScreenContext() override;

  // |Surface|
  bool ResourceContextMakeCurrent() const override;

  // |Surface|
  bool ClearCurrentContext() const override;

  // |SurfaceGlDelegate|
  bool GLContextMakeCurrent() const override;

  // |SurfaceGlDelegate|
  bool GLContextClearCurrent() const override;

  // |SurfaceGlDelegate|
  bool GLContextPresent(uint32_t fbo_id) const override;

  // |SurfaceGlDelegate|
  uint32_t GLContextFBO() const override;

  // |SurfaceGlDelegate|
  void* GlProcResolver(const char* name) const override;

 private:
  std::unique_ptr<ContextEglX11> context_;
  NativeWindow<xcb_window_t>* native_window_;
  std::unique_ptr<LinuxesEGLSurface> onscreen_surface_;
  std::unique_ptr<LinuxesEGLSurface> offscreen_surface_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_X11_H_
