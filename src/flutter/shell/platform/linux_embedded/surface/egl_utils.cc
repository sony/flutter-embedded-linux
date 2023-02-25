
// Copyright 2023 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/surface/egl_utils.h"

#include <EGL/egl.h>

#include <cstring>
#include <string>
#include <vector>

namespace flutter {

std::string get_egl_error_cause() {
  static const std::vector<std::pair<EGLint, std::string>> table = {
      {EGL_SUCCESS, "EGL_SUCCESS"},
      {EGL_NOT_INITIALIZED, "EGL_NOT_INITIALIZED"},
      {EGL_BAD_ACCESS, "EGL_BAD_ACCESS"},
      {EGL_BAD_ALLOC, "EGL_BAD_ALLOC"},
      {EGL_BAD_ATTRIBUTE, "EGL_BAD_ATTRIBUTE"},
      {EGL_BAD_CONTEXT, "EGL_BAD_CONTEXT"},
      {EGL_BAD_CONFIG, "EGL_BAD_CONFIG"},
      {EGL_BAD_CURRENT_SURFACE, "EGL_BAD_CURRENT_SURFACE"},
      {EGL_BAD_DISPLAY, "EGL_BAD_DISPLAY"},
      {EGL_BAD_SURFACE, "EGL_BAD_SURFACE"},
      {EGL_BAD_MATCH, "EGL_BAD_MATCH"},
      {EGL_BAD_PARAMETER, "EGL_BAD_PARAMETER"},
      {EGL_BAD_NATIVE_PIXMAP, "EGL_BAD_NATIVE_PIXMAP"},
      {EGL_BAD_NATIVE_WINDOW, "EGL_BAD_NATIVE_WINDOW"},
      {EGL_CONTEXT_LOST, "EGL_CONTEXT_LOST"},
  };

  auto egl_error = eglGetError();
  for (auto t : table) {
    if (egl_error == t.first) {
      return std::string("eglGetError: " + t.second);
    }
  }
  return nullptr;
}

// Auxiliary function used to check if the given list of extensions contains the
// requested extension name.
bool has_egl_extension(const char* extensions, const char* name) {
  const char* r = std::strstr(extensions, name);
  auto len = std::strlen(name);

  // check that the extension name is terminated by space or null terminator
  return r != nullptr && (r[len] == ' ' || r[len] == 0);
}

}  // namespace flutter
