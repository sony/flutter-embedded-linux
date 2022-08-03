// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_EXTERNAL_TEXTURE_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_EXTERNAL_TEXTURE_H_

#include "flutter/shell/platform/embedder/embedder.h"

#ifdef USE_GLES3
#include <GLES3/gl32.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

namespace flutter {

typedef void (*glGenTexturesProc)(GLsizei n, GLuint* textures);
typedef void (*glDeleteTexturesProc)(GLsizei n, const GLuint* textures);
typedef void (*glBindTextureProc)(GLenum target, GLuint texture);
typedef void (*glTexParameteriProc)(GLenum target, GLenum pname, GLint param);
typedef void (*glTexImage2DProc)(GLenum target,
                                 GLint level,
                                 GLint internalformat,
                                 GLsizei width,
                                 GLsizei height,
                                 GLint border,
                                 GLenum format,
                                 GLenum type,
                                 const void* data);

// A struct containing pointers to resolved gl* functions.
struct GlProcs {
  glGenTexturesProc glGenTextures;
  glDeleteTexturesProc glDeleteTextures;
  glBindTextureProc glBindTexture;
  glTexParameteriProc glTexParameteri;
  glTexImage2DProc glTexImage2D;
  bool valid;
};

// Abstract external texture.
class ExternalTexture {
 public:
  virtual ~ExternalTexture() = default;

  // Returns the unique id of this texture.
  int64_t texture_id() const { return reinterpret_cast<int64_t>(this); };

  // Attempts to populate the specified |opengl_texture| with texture details
  // such as the name, width, height and the pixel format.
  // Returns true on success.
  virtual bool PopulateTexture(size_t width,
                               size_t height,
                               FlutterOpenGLTexture* opengl_texture) = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_EXTERNAL_TEXTURE_H_
