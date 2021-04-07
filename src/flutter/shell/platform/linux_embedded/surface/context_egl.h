// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_H_

#include <EGL/egl.h>

#include <memory>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/egl_utils.h"
#include "flutter/shell/platform/linux_embedded/surface/environment_egl.h"
#include "flutter/shell/platform/linux_embedded/surface/linuxes_egl_surface.h"
#include "flutter/shell/platform/linux_embedded/window/native_window.h"

namespace flutter {

template <typename D, typename W>
class ContextEgl {
 public:
  ContextEgl(std::unique_ptr<EnvironmentEgl<D>> environment,
             int32_t egl_surface_type = EGL_WINDOW_BIT)
      : environment_(std::move(environment)), config_(nullptr) {
    EGLint config_count = 0;
    const EGLint attribs[] = {
        // clang-format off
        EGL_SURFACE_TYPE,    egl_surface_type,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_ALPHA_SIZE,      8,
        EGL_DEPTH_SIZE,      0,
        EGL_STENCIL_SIZE,    0,
        EGL_NONE
        // clang-format on
    };
    if (eglChooseConfig(environment_->Display(), attribs, &config_, 1,
                        &config_count) != EGL_TRUE) {
      LINUXES_LOG(ERROR) << "Failed to choose EGL surface config: "
                         << get_egl_error_cause();
      return;
    }

    if (config_count == 0 || config_ == nullptr) {
      LINUXES_LOG(ERROR) << "No matching configs: " << get_egl_error_cause();
      return;
    }

    {
      const EGLint attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
      context_ = eglCreateContext(environment_->Display(), config_,
                                  EGL_NO_CONTEXT, attribs);
      if (context_ == EGL_NO_CONTEXT) {
        LINUXES_LOG(ERROR) << "Failed to create an onscreen context: "
                           << get_egl_error_cause();
        return;
      }

      resource_context_ =
          eglCreateContext(environment_->Display(), config_, context_, attribs);
      if (resource_context_ == EGL_NO_CONTEXT) {
        LINUXES_LOG(ERROR) << "Failed to create an offscreen resouce context: "
                           << get_egl_error_cause();
        return;
      }
    }

    valid_ = true;
  }

  ~ContextEgl() = default;

  virtual std::unique_ptr<LinuxesEGLSurface> CreateOnscreenSurface(
      NativeWindow<W>* window) const {
    const EGLint attribs[] = {EGL_NONE};
    EGLSurface surface = eglCreateWindowSurface(
        environment_->Display(), config_, window->Window(), attribs);
    if (surface == EGL_NO_SURFACE) {
      LINUXES_LOG(ERROR) << "Failed to create EGL window surface: "
                         << get_egl_error_cause();
    }
    return std::make_unique<LinuxesEGLSurface>(surface, environment_->Display(),
                                               context_);
  }

  std::unique_ptr<LinuxesEGLSurface> CreateOffscreenSurface(
      NativeWindow<W>* window_resource) const {
#if defined(DISPLAY_BACKEND_TYPE_X11) || \
    defined(DISPLAY_BACKEND_TYPE_DRM_EGLSTREAM)
    const EGLint attribs[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE};
    EGLSurface surface =
        eglCreatePbufferSurface(environment_->Display(), config_, attribs);
    if (surface == EGL_NO_SURFACE) {
      LINUXES_LOG(WARNING) << "Failed to create EGL off-screen surface."
                           << "(" << get_egl_error_cause() << ")";
    }
#else
    // eglCreatePbufferSurface isn't supported on both Wayland and GBM.
    // Therefore, we neet to create a dummy wl_egl_window when we use Wayland.
    const EGLint attribs[] = {EGL_NONE};
    EGLSurface surface = eglCreateWindowSurface(
        environment_->Display(), config_, window_resource->Window(), attribs);
    if (surface == EGL_NO_SURFACE) {
      LINUXES_LOG(WARNING) << "Failed to create EGL off-screen surface."
                           << "(" << get_egl_error_cause() << ")";
    }
#endif
    return std::make_unique<LinuxesEGLSurface>(surface, environment_->Display(),
                                               resource_context_);
  }

  bool IsValid() const { return valid_; }

  bool ClearCurrent() const {
    if (eglGetCurrentContext() != context_) {
      return true;
    }
    if (eglMakeCurrent(environment_->Display(), EGL_NO_SURFACE, EGL_NO_SURFACE,
                       EGL_NO_CONTEXT) != EGL_TRUE) {
      LINUXES_LOG(ERROR) << "Failed to clear EGL context: "
                         << get_egl_error_cause();
      return false;
    }
    return true;
  }

  void* GlProcResolver(const char* name) const {
    auto address = eglGetProcAddress(name);
    if (!address) {
      LINUXES_LOG(ERROR) << "Failed eglGetProcAddress: " << name;
      return nullptr;
    }
    return reinterpret_cast<void*>(address);
  }

 protected:
  std::unique_ptr<EnvironmentEgl<D>> environment_;
  EGLConfig config_;
  EGLContext context_;
  EGLContext resource_context_;
  bool valid_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_H_