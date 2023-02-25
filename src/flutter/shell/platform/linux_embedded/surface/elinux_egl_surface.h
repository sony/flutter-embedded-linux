// Copyright 2023 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ELINUX_EGL_SURFACE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ELINUX_EGL_SURFACE_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <array>
#include <list>
#include <unordered_map>

#include "flutter/shell/platform/embedder/embedder.h"

namespace flutter {

class ELinuxEGLSurface {
 public:
  // Note that EGLSurface will be destroyed in this class's destructor.
  ELinuxEGLSurface(EGLSurface surface, EGLDisplay display, EGLContext context);
  ~ELinuxEGLSurface();

  bool IsValid() const;

  void SurfaceResize(const size_t width_px, const size_t height_px);

  bool MakeCurrent() const;

  bool SwapBuffers() const;

  bool SwapBuffers(const FlutterPresentInfo* info);

  void PopulateExistingDamage(const intptr_t fbo_id,
                              FlutterDamage* existing_damage);

 private:
  // Auxiliary function used to transform a FlutterRect into the format that is
  // expected by the EGL functions (i.e. array of EGLint).
  std::array<EGLint, 4> RectToInts(const FlutterRect rect);

  EGLDisplay display_;
  EGLSurface surface_;
  EGLContext context_;

  size_t width_px_;
  size_t height_px_;

  PFNEGLSETDAMAGEREGIONKHRPROC eglSetDamageRegionKHR_ = nullptr;
  PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC eglSwapBuffersWithDamageEXT_ = nullptr;

  // Keeps track of the most recent frame damages so that existing damage can
  // be easily computed.
  std::list<FlutterRect> damage_history_;

  // Keeps track of the existing damage associated with each FBO ID
  std::unordered_map<intptr_t, FlutterRect*> existing_damage_map_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ELINUX_EGL_SURFACE_H_
