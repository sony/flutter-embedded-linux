// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_WAYLAND_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_WAYLAND_H_

#include <wayland-client.h>

#include <memory>

#include "flutter/shell/platform/linux_embedded/surface/context_egl_wayland.h"
#include "flutter/shell/platform/linux_embedded/surface/linuxes_egl_surface.h"
#include "flutter/shell/platform/linux_embedded/surface/linuxes_surface.h"
#include "flutter/shell/platform/linux_embedded/surface/linuxes_surface_gl_delegate.h"

namespace flutter {

class SurfaceGlWayland final : public Surface<wl_egl_window>,
                               public SurfaceGlDelegate {
 public:
  SurfaceGlWayland(std::unique_ptr<ContextEglWayland> context);
  ~SurfaceGlWayland() = default;

  // |Surface|
  bool IsValid() const override;

  // |Surface|
  bool SetNativeWindow(NativeWindow<wl_egl_window>* window) override;

  bool SetNativeWindowResource(
      std::unique_ptr<NativeWindow<wl_egl_window>> window);

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
  std::unique_ptr<ContextEglWayland> context_;
  NativeWindow<wl_egl_window>* native_window_;
  std::unique_ptr<NativeWindow<wl_egl_window>> native_window_resource_;
  std::unique_ptr<LinuxesEGLSurface> onscreen_surface_;
  std::unique_ptr<LinuxesEGLSurface> offscreen_surface_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_WAYLAND_H_