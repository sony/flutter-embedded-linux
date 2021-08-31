// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_RENDERER_SHADER_PROGRAM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_RENDERER_SHADER_PROGRAM_H_

#ifdef USE_GLES3
#include <GLES3/gl32.h>
#else
#include <GLES2/gl2.h>
#endif

#include <memory>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/window/renderer/elinux_shader_context.h"

namespace flutter {

class ELinuxShaderProgram {
 public:
  ELinuxShaderProgram(std::unique_ptr<ELinuxShaderContext> vertex_shader,
                      std::unique_ptr<ELinuxShaderContext> fragment_shader);
  ~ELinuxShaderProgram();

  GLuint Program() const { return program_; }

 private:
  GLuint program_;
  std::unique_ptr<ELinuxShaderContext> vertex_shader_;
  std::unique_ptr<ELinuxShaderContext> fragment_shader_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_RENDERER_SHADER_PROGRAM_H_
