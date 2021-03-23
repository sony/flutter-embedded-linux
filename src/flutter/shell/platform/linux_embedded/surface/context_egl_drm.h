// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_DRM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_DRM_H_

#include <gbm.h>

#include <functional>

#include "flutter/shell/platform/linux_embedded/surface/context_egl.h"

namespace flutter {

using ContextEglDrm = ContextEgl<gbm_device, gbm_surface>;

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_DRM_H_