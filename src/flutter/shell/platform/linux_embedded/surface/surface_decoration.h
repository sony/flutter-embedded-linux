// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_DECORATION_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_DECORATION_H_

#include <memory>

#include "flutter/shell/platform/linux_embedded/surface/context_egl.h"
#include "flutter/shell/platform/linux_embedded/surface/elinux_egl_surface.h"
#include "flutter/shell/platform/linux_embedded/surface/surface_gl_delegate.h"
#include "flutter/shell/platform/linux_embedded/window/native_window.h"

namespace flutter {

// Surface
class SurfaceDecoration : public SurfaceGlDelegate {
 public:
  SurfaceDecoration(std::unique_ptr<ContextEgl> context);
  ~SurfaceDecoration() = default;

  // Shows a surface is valid or not.
  bool IsValid() const;

  // Sets a netive platform's window.
  bool SetNativeWindow(NativeWindow* window);

  // Changes an decoration surface size.
  bool Resize(const size_t width, const size_t height);

  // Clears and destroys current decoration context.
  void DestroyContext();

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

 protected:
  std::unique_ptr<ContextEgl> context_;
  NativeWindow* native_window_ = nullptr;
  std::unique_ptr<ELinuxEGLSurface> surface_ = nullptr;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_DECORATION_H_
