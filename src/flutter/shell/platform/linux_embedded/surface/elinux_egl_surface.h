// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ELINUX_EGL_SURFACE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ELINUX_EGL_SURFACE_H_

#include <EGL/egl.h>

namespace flutter {

class ELinuxEGLSurface {
 public:
  // Note that EGLSurface will be destroyed in this class's destructor.
  ELinuxEGLSurface(EGLSurface surface, EGLDisplay display, EGLContext context);
  ~ELinuxEGLSurface();

  bool IsValid() const;

  bool MakeCurrent() const;

  bool SwapBuffers() const;

 private:
  EGLDisplay display_;
  EGLSurface surface_;
  EGLContext context_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ELINUX_EGL_SURFACE_H_
