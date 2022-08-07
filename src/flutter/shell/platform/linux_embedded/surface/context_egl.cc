// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/surface/context_egl.h"

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/egl_utils.h"

namespace flutter {

ContextEgl::ContextEgl(std::unique_ptr<EnvironmentEgl> environment,
                       EGLint egl_surface_type)
    : environment_(std::move(environment)), config_(nullptr) {
  EGLint config_count = 0;
  const EGLint attribs[] = {
      // clang-format off
      EGL_SURFACE_TYPE,    egl_surface_type,
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
      EGL_RED_SIZE,        8,
      EGL_GREEN_SIZE,      8,
      EGL_BLUE_SIZE,       8,
#if defined(ENABLE_EGL_ALPHA_COMPONENT_OF_COLOR_BUFFER)
      EGL_ALPHA_SIZE,      8,
#endif
      EGL_DEPTH_SIZE,      0,
      EGL_STENCIL_SIZE,    0,
      EGL_NONE
      // clang-format on
  };
  if (eglChooseConfig(environment_->Display(), attribs, &config_, 1,
                      &config_count) != EGL_TRUE) {
    ELINUX_LOG(ERROR) << "Failed to choose EGL surface config: "
                      << get_egl_error_cause();
    return;
  }

  if (config_count == 0 || config_ == nullptr) {
    ELINUX_LOG(ERROR) << "No matching configs: " << get_egl_error_cause();
    return;
  }

  {
    const EGLint attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    context_ = eglCreateContext(environment_->Display(), config_,
                                EGL_NO_CONTEXT, attribs);
    if (context_ == EGL_NO_CONTEXT) {
      ELINUX_LOG(ERROR) << "Failed to create an onscreen context: "
                        << get_egl_error_cause();
      return;
    }

    resource_context_ =
        eglCreateContext(environment_->Display(), config_, context_, attribs);
    if (resource_context_ == EGL_NO_CONTEXT) {
      ELINUX_LOG(ERROR) << "Failed to create an offscreen resouce context: "
                        << get_egl_error_cause();
      return;
    }
  }

  valid_ = true;
}

std::unique_ptr<ELinuxEGLSurface> ContextEgl::CreateOnscreenSurface(
    NativeWindow* window) const {
  const EGLint attribs[] = {EGL_NONE};
  EGLSurface surface = eglCreateWindowSurface(environment_->Display(), config_,
                                              window->Window(), attribs);
  if (surface == EGL_NO_SURFACE) {
    ELINUX_LOG(ERROR) << "Failed to create EGL window surface: "
                      << get_egl_error_cause();
  }
  return std::make_unique<ELinuxEGLSurface>(surface, environment_->Display(),
                                            context_);
}

std::unique_ptr<ELinuxEGLSurface> ContextEgl::CreateOffscreenSurface(
    NativeWindow* window) const {
#if defined(DISPLAY_BACKEND_TYPE_X11) || \
    defined(DISPLAY_BACKEND_TYPE_DRM_EGLSTREAM)
  const EGLint attribs[] = {
      // clang-format off
      EGL_WIDTH, 1,
      EGL_HEIGHT, 1,
      EGL_NONE
      // clang-format on
  };
  EGLSurface surface =
      eglCreatePbufferSurface(environment_->Display(), config_, attribs);
  if (surface == EGL_NO_SURFACE) {
    ELINUX_LOG(WARNING) << "Failed to create EGL off-screen surface."
                        << "(" << get_egl_error_cause() << ")";
  }
#else
  // eglCreatePbufferSurface isn't supported on both Wayland and GBM.
  // Therefore, we neet to create a dummy wl_egl_window when we use Wayland.
  const EGLint attribs[] = {EGL_NONE};
  EGLSurface surface = eglCreateWindowSurface(
      environment_->Display(), config_, window->WindowOffscreen(), attribs);
  if (surface == EGL_NO_SURFACE) {
    ELINUX_LOG(WARNING) << "Failed to create EGL off-screen surface."
                        << "(" << get_egl_error_cause() << ")";
  }
#endif
  return std::make_unique<ELinuxEGLSurface>(surface, environment_->Display(),
                                            resource_context_);
}

bool ContextEgl::IsValid() const {
  return valid_;
}

bool ContextEgl::ClearCurrent() const {
  if (eglGetCurrentContext() != context_) {
    return true;
  }
  if (eglMakeCurrent(environment_->Display(), EGL_NO_SURFACE, EGL_NO_SURFACE,
                     EGL_NO_CONTEXT) != EGL_TRUE) {
    ELINUX_LOG(ERROR) << "Failed to clear EGL context: "
                      << get_egl_error_cause();
    return false;
  }
  return true;
}

void* ContextEgl::GlProcResolver(const char* name) const {
  auto address = eglGetProcAddress(name);
  if (!address) {
    ELINUX_LOG(ERROR) << "Failed eglGetProcAddress: " << name;
    return nullptr;
  }
  return reinterpret_cast<void*>(address);
}

EGLint ContextEgl::GetAttrib(EGLint attribute) {
  EGLint value;
  eglGetConfigAttrib(environment_->Display(), config_, attribute, &value);
  return value;
}

}  // namespace flutter
