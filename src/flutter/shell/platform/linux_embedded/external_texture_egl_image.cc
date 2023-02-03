// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/external_texture_egl_image.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace flutter {

struct ExternalTextureEGLImageState {
  GLuint gl_texture = 0;
};

ExternalTextureEGLImage::ExternalTextureEGLImage(
    FlutterDesktopEGLImageTextureCallback texture_callback,
    void* user_data,
    const GlProcs& gl_procs)
    : state_(std::make_unique<ExternalTextureEGLImageState>()),
      texture_callback_(texture_callback),
      user_data_(user_data),
      gl_(gl_procs) {}

ExternalTextureEGLImage::~ExternalTextureEGLImage() {
  if (state_->gl_texture != 0) {
    gl_.glDeleteTextures(1, &state_->gl_texture);
  }
}

bool ExternalTextureEGLImage::PopulateTexture(
    size_t width,
    size_t height,
    FlutterOpenGLTexture* opengl_texture) {
  if (!GetEGLImage(width, height, eglGetCurrentDisplay(),
                   eglGetCurrentContext())) {
    return false;
  }

  // Populate the texture object used by the engine.
  opengl_texture->target = GL_TEXTURE_2D;
  opengl_texture->name = state_->gl_texture;
#ifdef USE_GLES3
  opengl_texture->format = GL_RGBA8;
#else
  opengl_texture->format = GL_RGBA8_OES;
#endif
  opengl_texture->destruction_callback = nullptr;
  opengl_texture->user_data = nullptr;
  opengl_texture->width = width;
  opengl_texture->height = height;

  return true;
}

bool ExternalTextureEGLImage::GetEGLImage(size_t& width,
                                          size_t& height,
                                          void* egl_display,
                                          void* egl_context) {
  using namespace std;

  const FlutterDesktopEGLImage* egl_image =
      texture_callback_(width, height, egl_display, egl_context, user_data_);
  if (!egl_image || !egl_image->egl_image) {
    return false;
  }
  width = egl_image->width;
  height = egl_image->height;

  if (state_->gl_texture == 0) {
    gl_.glGenTextures(1, &state_->gl_texture);

    gl_.glBindTexture(GL_TEXTURE_2D, state_->gl_texture);
    gl_.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl_.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl_.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl_.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    gl_.glBindTexture(GL_TEXTURE_2D, state_->gl_texture);
  }
  gl_.glEGLImageTargetTexture2DOES(GL_TEXTURE_2D,
                                   (EGLImageKHR)egl_image->egl_image);

  if (egl_image->release_callback) {
    egl_image->release_callback(egl_image->release_context);
  }
  return true;
}

}  // namespace flutter
