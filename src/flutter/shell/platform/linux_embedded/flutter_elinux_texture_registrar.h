// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_FLUTTER_ELINUX_TEXTURE_REGISTRAR_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_FLUTTER_ELINUX_TEXTURE_REGISTRAR_H_

#include <memory>
#include <mutex>
#include <unordered_map>

#include "flutter/shell/platform/linux_embedded/external_texture_gl.h"

namespace flutter {

class FlutterELinuxEngine;

// An object managing the registration of an external texture.
// Thread safety: All member methods are thread safe.
class FlutterELinuxTextureRegistrar {
 public:
  explicit FlutterELinuxTextureRegistrar(FlutterELinuxEngine* engine);

  // Registers a texture described by the given |texture_info| object.
  // Returns the non-zero, positive texture id or -1 on error.
  int64_t RegisterTexture(const FlutterDesktopTextureInfo* texture_info);

  // Attempts to unregister the texture identified by |texture_id|.
  // Returns true if the texture was successfully unregistered.
  bool UnregisterTexture(int64_t texture_id);

  // Notifies the engine about a new frame being available.
  // Returns true on success.
  bool MarkTextureFrameAvailable(int64_t texture_id);

  // Attempts to populate the given |texture| by copying the
  // contents of the texture identified by |texture_id|.
  // Returns true on success.
  bool PopulateTexture(int64_t texture_id, size_t width, size_t height,
                       FlutterOpenGLTexture* texture);

 private:
  FlutterELinuxEngine* engine_ = nullptr;

  // All registered textures, keyed by their IDs.
  std::unordered_map<int64_t, std::unique_ptr<flutter::ExternalTextureGL>>
      textures_;
  std::mutex map_mutex_;
};

};  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_FLUTTER_ELINUX_TEXTURE_REGISTRAR_H_
