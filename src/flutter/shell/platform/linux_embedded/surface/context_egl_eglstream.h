// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_EGLSTREAM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_EGLSTREAM_H_

#include <functional>

#include "flutter/shell/platform/linux_embedded/surface/context_egl.h"

namespace flutter {

using ContextEglEglstream = ContextEgl<int, void>;

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_EGLSTREAM_H_