// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_EXTERNAL_TEXTURE_EGL_IMAGE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_EXTERNAL_TEXTURE_EGL_IMAGE_H_

#include <stdint.h>

#include <memory>

#include "flutter/shell/platform/common/public/flutter_texture_registrar.h"

#include "flutter/shell/platform/linux_embedded/external_texture.h"

namespace flutter {

typedef struct ExternalTextureEGLImageState ExternalTextureEGLImageState;

// An abstraction of an EGL Image based texture.
class ExternalTextureEGLImage : public ExternalTexture {
 public:
  ExternalTextureEGLImage(
      FlutterDesktopEGLImageTextureCallback texture_callback,
      void* user_data,
      const GlProcs& gl_procs);

  virtual ~ExternalTextureEGLImage();

  // |ExternalTexture|
  bool PopulateTexture(size_t width,
                       size_t height,
                       FlutterOpenGLTexture* opengl_texture) override;

 private:
  // Attempts to get the EGLImage returned by |texture_callback_| to
  // OpenGL.
  // The |width| and |height| will be set to the actual bounds of the EGLImage
  // Returns true on success or false if the EGLImage returned
  // by |texture_callback_| was invalid.
  bool GetEGLImage(size_t& width,
                   size_t& height,
                   void* egl_display,
                   void* egl_context);

  std::unique_ptr<ExternalTextureEGLImageState> state_;
  FlutterDesktopEGLImageTextureCallback texture_callback_ = nullptr;
  void* const user_data_ = nullptr;
  const GlProcs& gl_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_EXTERNAL_TEXTURE_EGL_IMAGE_H_
