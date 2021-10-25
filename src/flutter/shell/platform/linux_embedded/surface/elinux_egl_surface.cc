// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/surface/elinux_egl_surface.h"

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/egl_utils.h"

// The parameter interval specifies the minimum number of video frames 
//  that are displayed before a buffer swap will occur
// If interval is set to a value of 0, buffer swaps are not synchronized 
//   to a video frame, and the swap happens as soon as all rendering 
//   commands outstanding for the current context are complete.
#if defined(DISPLAY_BACKEND_TYPE_WAYLAND)
#ifndef BUF_NUM_SWAP_INTERVAL
#define BUF_NUM_SWAP_INTERVAL 0
#endif
#endif

namespace flutter {

ELinuxEGLSurface::ELinuxEGLSurface(EGLSurface surface, EGLDisplay display,
                                   EGLContext context)
    : surface_(surface), display_(display), context_(context){};

ELinuxEGLSurface::~ELinuxEGLSurface() {
  if (surface_ != EGL_NO_SURFACE) {
    if (eglDestroySurface(display_, surface_) != EGL_TRUE) {
      ELINUX_LOG(ERROR) << "Failed to destory surface: "
                        << get_egl_error_cause();
    }
    surface_ = EGL_NO_SURFACE;
  }
}

bool ELinuxEGLSurface::IsValid() const { return surface_ != EGL_NO_SURFACE; }

bool ELinuxEGLSurface::MakeCurrent() const {
  if (eglMakeCurrent(display_, surface_, surface_, context_) != EGL_TRUE) {
    ELINUX_LOG(ERROR) << "Failed to make the EGL context current: "
                      << get_egl_error_cause();
    return false;
  }

#if defined(DISPLAY_BACKEND_TYPE_WAYLAND)
  // Non-blocking when swappipping buffers on Wayland.
  if (eglSwapInterval(display_, BUF_NUM_SWAP_INTERVAL) != EGL_TRUE) {
    ELINUX_LOG(ERROR) << "Failed to eglSwapInterval(Free): "
                      << get_egl_error_cause();
  }
#endif

  return true;
}

bool ELinuxEGLSurface::SwapBuffers() const {
  if (eglSwapBuffers(display_, surface_) != EGL_TRUE) {
    ELINUX_LOG(ERROR) << "Failed to swap the EGL buffer: "
                      << get_egl_error_cause();
    return false;
  }
  return true;
}

}  // namespace flutter
