// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_DRM_EGLSTREAM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_DRM_EGLSTREAM_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/context_egl.h"
#include "flutter/shell/platform/linux_embedded/surface/environment_egl_drm_eglstream.h"
#include "flutter/shell/platform/linux_embedded/surface/linuxes_egl_surface.h"
#include "flutter/shell/platform/linux_embedded/window/native_window_drm_eglstream.h"

namespace flutter {

class ContextEglDrmEglstream : public ContextEgl<int, uint32_t> {
 public:
  ContextEglDrmEglstream(
      std::unique_ptr<EnvironmentEglDrmEglstream<int>> environment)
      : ContextEgl(std::move(environment)) {
    if (!SetEglExtensionFunctionPointers()) {
      LINUXES_LOG(ERROR) << "Failed to set extension function pointers";
      return;
    }

    EGLint config_count = 0;
    const EGLint attribs[] = {
        // clang-format off
        EGL_SURFACE_TYPE,    EGL_STREAM_BIT_KHR,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE,        1,
        EGL_GREEN_SIZE,      1,
        EGL_BLUE_SIZE,       1,
        EGL_ALPHA_SIZE,      0,
        EGL_DEPTH_SIZE,      1,
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

  ~ContextEglDrmEglstream() = default;

  std::unique_ptr<LinuxesEGLSurface> CreateOnscreenSurface(
      NativeWindow<uint32_t>* window) const override {
    EGLint layer_count = 0;
    EGLOutputLayerEXT layer;
    EGLAttrib layer_attribs[] = {
        EGL_DRM_PLANE_EXT,
        static_cast<NativeWindowDrmEglstream*>(window)->PlaneId(),
        EGL_NONE,
    };

    if (eglGetOutputLayersEXT_(environment_->Display(), layer_attribs, &layer,
                               1, &layer_count) != EGL_TRUE) {
      LINUXES_LOG(ERROR) << "Failed to get EGL output layers";
    }
    if (layer_count == 0 || layer == nullptr) {
      LINUXES_LOG(ERROR) << "No matching layers";
    }

    EGLint stream_attribs[] = {EGL_NONE};
    auto stream = eglCreateStreamKHR_(environment_->Display(), stream_attribs);
    if (stream == EGL_NO_STREAM_KHR) {
      LINUXES_LOG(ERROR) << "Failed to create EGL stream";
    }

    if (eglStreamConsumerOutputEXT_(environment_->Display(), stream, layer) !=
        EGL_TRUE) {
      LINUXES_LOG(ERROR) << "Failed to create EGL stream consumer output";
    }

    EGLint surface_attribs[] = {
        EGL_WIDTH, static_cast<NativeWindowDrmEglstream*>(window)->Width(),
        EGL_HEIGHT, static_cast<NativeWindowDrmEglstream*>(window)->Height(),
        EGL_NONE};
    auto surface = eglCreateStreamProducerSurfaceKHR_(
        environment_->Display(), config_, stream, surface_attribs);
    if (surface == EGL_NO_SURFACE) {
      LINUXES_LOG(ERROR) << "Failed to create EGL stream producer surface";
    }
    return std::make_unique<LinuxesEGLSurface>(surface, environment_->Display(),
                                               context_);
  }

 private:
  bool SetEglExtensionFunctionPointers() {
    eglGetOutputLayersEXT_ = (PFNEGLGETOUTPUTLAYERSEXTPROC)eglGetProcAddress(
        "eglGetOutputLayersEXT");
    eglCreateStreamKHR_ =
        (PFNEGLCREATESTREAMKHRPROC)eglGetProcAddress("eglCreateStreamKHR");
    eglStreamConsumerOutputEXT_ =
        (PFNEGLSTREAMCONSUMEROUTPUTEXTPROC)eglGetProcAddress(
            "eglStreamConsumerOutputEXT");
    eglCreateStreamProducerSurfaceKHR_ =
        (PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC)eglGetProcAddress(
            "eglCreateStreamProducerSurfaceKHR");

    if (!eglGetOutputLayersEXT_ || !eglCreateStreamKHR_ ||
        !eglStreamConsumerOutputEXT_ || !eglCreateStreamProducerSurfaceKHR_) {
      return false;
    }
    return true;
  }

  PFNEGLGETOUTPUTLAYERSEXTPROC eglGetOutputLayersEXT_;
  PFNEGLCREATESTREAMKHRPROC eglCreateStreamKHR_;
  PFNEGLSTREAMCONSUMEROUTPUTEXTPROC eglStreamConsumerOutputEXT_;
  PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC eglCreateStreamProducerSurfaceKHR_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_DRM_EGLSTREAM_H_