// Copyright 2023 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_EGL_UTILS_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_EGL_UTILS_H_

#include <string>

namespace flutter {

std::string get_egl_error_cause();
bool has_egl_extension(const char* extensions, const char* name);

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_EGL_UTILS_H_