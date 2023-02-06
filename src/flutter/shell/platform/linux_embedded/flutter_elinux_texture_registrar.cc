// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/flutter_elinux_texture_registrar.h"

#include <iostream>
#include <mutex>

#include "flutter/shell/platform/embedder/embedder_struct_macros.h"
#include "flutter/shell/platform/linux_embedded/external_texture_egl_image.h"
#include "flutter/shell/platform/linux_embedded/external_texture_pixelbuffer.h"
#include "flutter/shell/platform/linux_embedded/flutter_elinux_engine.h"
#include "flutter/shell/platform/linux_embedded/flutter_elinux_view.h"

namespace {
static constexpr int64_t kInvalidTexture = -1;
}

namespace flutter {

FlutterELinuxTextureRegistrar::FlutterELinuxTextureRegistrar(
    FlutterELinuxEngine* engine,
    const GlProcs& gl_procs)
    : engine_(engine), gl_procs_(gl_procs) {}

int64_t FlutterELinuxTextureRegistrar::RegisterTexture(
    const FlutterDesktopTextureInfo* texture_info) {
  if (!gl_procs_.valid) {
    return kInvalidTexture;
  }

  if (texture_info->type == kFlutterDesktopPixelBufferTexture) {
    if (!texture_info->pixel_buffer_config.callback) {
      std::cerr << "Invalid pixel buffer texture callback." << std::endl;
      return kInvalidTexture;
    }

    return EmplaceTexture(std::make_unique<flutter::ExternalTexturePixelBuffer>(
        texture_info->pixel_buffer_config.callback,
        texture_info->pixel_buffer_config.user_data, gl_procs_));
  } else if (texture_info->type == kFlutterDesktopEGLImageTexture) {
    if (!texture_info->egl_image_config.callback) {
      std::cerr << "Invalid EGLImage texture callback." << std::endl;
      return kInvalidTexture;
    }

    return EmplaceTexture(std::make_unique<flutter::ExternalTextureEGLImage>(
        texture_info->egl_image_config.callback,
        texture_info->egl_image_config.user_data, gl_procs_));
  } else if (texture_info->type == kFlutterDesktopGpuSurfaceTexture) {
    std::cerr << "GpuSurfaceTexture is not yet supported." << std::endl;
    return kInvalidTexture;
  }

  std::cerr << "Attempted to register texture of unsupport type." << std::endl;
  return kInvalidTexture;
}

int64_t FlutterELinuxTextureRegistrar::EmplaceTexture(
    std::unique_ptr<ExternalTexture> texture) {
  int64_t texture_id = texture->texture_id();
  {
    std::lock_guard<std::mutex> lock(map_mutex_);
    textures_[texture_id] = std::move(texture);
  }

  engine_->task_runner()->RunNowOrPostTask([engine = engine_, texture_id]() {
    engine->RegisterExternalTexture(texture_id);
  });

  return texture_id;
}

bool FlutterELinuxTextureRegistrar::UnregisterTexture(int64_t texture_id) {
  {
    std::lock_guard<std::mutex> lock(map_mutex_);
    auto it = textures_.find(texture_id);
    if (it == textures_.end()) {
      return false;
    }
    textures_.erase(it);
  }

  engine_->task_runner()->RunNowOrPostTask([engine = engine_, texture_id]() {
    engine->UnregisterExternalTexture(texture_id);
  });
  return true;
}

bool FlutterELinuxTextureRegistrar::MarkTextureFrameAvailable(
    int64_t texture_id) {
  engine_->task_runner()->RunNowOrPostTask([engine = engine_, texture_id]() {
    engine->MarkExternalTextureFrameAvailable(texture_id);
  });
  return true;
}

bool FlutterELinuxTextureRegistrar::PopulateTexture(
    int64_t texture_id,
    size_t width,
    size_t height,
    FlutterOpenGLTexture* opengl_texture) {
  flutter::ExternalTexture* texture;
  {
    std::lock_guard<std::mutex> lock(map_mutex_);
    auto it = textures_.find(texture_id);
    if (it == textures_.end()) {
      return false;
    }
    texture = it->second.get();
  }
  return texture->PopulateTexture(width, height, opengl_texture);
}

void FlutterELinuxTextureRegistrar::ResolveGlFunctions(GlProcs& procs) {
  procs.glGenTextures =
      reinterpret_cast<glGenTexturesProc>(eglGetProcAddress("glGenTextures"));
  procs.glDeleteTextures = reinterpret_cast<glDeleteTexturesProc>(
      eglGetProcAddress("glDeleteTextures"));
  procs.glBindTexture =
      reinterpret_cast<glBindTextureProc>(eglGetProcAddress("glBindTexture"));
  procs.glTexParameteri = reinterpret_cast<glTexParameteriProc>(
      eglGetProcAddress("glTexParameteri"));
  procs.glTexImage2D =
      reinterpret_cast<glTexImage2DProc>(eglGetProcAddress("glTexImage2D"));
  procs.glEGLImageTargetTexture2DOES =
      reinterpret_cast<glEGLImageTargetTexture2DOESProc>(
          eglGetProcAddress("glEGLImageTargetTexture2DOES"));
  procs.valid = procs.glGenTextures && procs.glDeleteTextures &&
                procs.glBindTexture && procs.glTexParameteri &&
                procs.glTexImage2D && procs.glEGLImageTargetTexture2DOES;
}

};  // namespace flutter
