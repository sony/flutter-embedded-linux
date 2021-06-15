// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_STREAM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_STREAM_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "flutter/shell/platform/linux_embedded/surface/context_egl.h"
#include "flutter/shell/platform/linux_embedded/surface/elinux_egl_surface.h"
#include "flutter/shell/platform/linux_embedded/surface/environment_egl_stream.h"

namespace flutter {

class ContextEglStream : public ContextEgl {
 public:
  ContextEglStream(std::unique_ptr<EnvironmentEglStream> environment);
  ~ContextEglStream() = default;

  // |ContextEgl|
  std::unique_ptr<ELinuxEGLSurface> CreateOnscreenSurface(
      NativeWindow* window) const override;

 private:
  bool SetEglExtensionFunctionPointers();

  PFNEGLGETOUTPUTLAYERSEXTPROC eglGetOutputLayersEXT_;
  PFNEGLCREATESTREAMKHRPROC eglCreateStreamKHR_;
  PFNEGLSTREAMCONSUMEROUTPUTEXTPROC eglStreamConsumerOutputEXT_;
  PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC eglCreateStreamProducerSurfaceKHR_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_STREAM_H_
