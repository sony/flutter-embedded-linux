// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_X11_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_X11_H_

#include <xcb/xcb.h>

#include <functional>

#include "flutter/shell/platform/linux_embedded/surface/context_egl.h"

namespace flutter {

using ContextEglX11 = ContextEgl<Display, xcb_window_t>;

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_X11_H_
