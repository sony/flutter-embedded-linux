
// Copyright 2023 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_DELEGATE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_DELEGATE_H_

#include <cstdint>

#include "flutter/shell/platform/embedder/embedder.h"

namespace flutter {

class SurfaceGlDelegate {
 public:
  virtual bool GLContextMakeCurrent() const = 0;

  virtual bool GLContextClearCurrent() const = 0;

  virtual bool GLContextPresent(uint32_t fbo_id) const = 0;

  virtual bool GLContextPresentWithInfo(
      const FlutterPresentInfo* info) const = 0;

  virtual void PopulateExistingDamage(const intptr_t fbo_id,
                                      FlutterDamage* existing_damage) const = 0;

  virtual uint32_t GLContextFBO() const = 0;

  virtual void* GlProcResolver(const char* name) const = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_SURFACE_GL_DELEGATE_H_
