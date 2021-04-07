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

class ContextEglDrmEglstream : public ContextEgl<void, uint32_t> {
 public:
  ContextEglDrmEglstream(
      std::unique_ptr<EnvironmentEglDrmEglstream> environment)
      : ContextEgl(std::move(environment), EGL_STREAM_BIT_KHR) {
    if (!SetEglExtensionFunctionPointers()) {
      LINUXES_LOG(ERROR) << "Failed to set extension function pointers";
      valid_ = false;
    }
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

    EGLint surface_attribs[] = {EGL_WIDTH, window->Width(), EGL_HEIGHT,
                                window->Height(), EGL_NONE};
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
    eglGetOutputLayersEXT_ = reinterpret_cast<PFNEGLGETOUTPUTLAYERSEXTPROC>(
        eglGetProcAddress("eglGetOutputLayersEXT"));
    eglCreateStreamKHR_ = reinterpret_cast<PFNEGLCREATESTREAMKHRPROC>(
        eglGetProcAddress("eglCreateStreamKHR"));
    eglStreamConsumerOutputEXT_ =
        reinterpret_cast<PFNEGLSTREAMCONSUMEROUTPUTEXTPROC>(
            eglGetProcAddress("eglStreamConsumerOutputEXT"));
    eglCreateStreamProducerSurfaceKHR_ =
        reinterpret_cast<PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC>(
            eglGetProcAddress("eglCreateStreamProducerSurfaceKHR"));

    return eglGetOutputLayersEXT_ && eglCreateStreamKHR_ &&
           eglStreamConsumerOutputEXT_ && eglCreateStreamProducerSurfaceKHR_;
  }

  PFNEGLGETOUTPUTLAYERSEXTPROC eglGetOutputLayersEXT_;
  PFNEGLCREATESTREAMKHRPROC eglCreateStreamKHR_;
  PFNEGLSTREAMCONSUMEROUTPUTEXTPROC eglStreamConsumerOutputEXT_;
  PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC eglCreateStreamProducerSurfaceKHR_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_DRM_EGLSTREAM_H_