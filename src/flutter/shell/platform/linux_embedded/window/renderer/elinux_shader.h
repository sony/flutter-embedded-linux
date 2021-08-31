// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_RENDERER_ELINUX_SHADER_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_RENDERER_ELINUX_SHADER_H_

#ifdef USE_GLES3
#include <GLES3/gl32.h>
#else
#include <GLES2/gl2.h>
#endif

#include <memory>
#include <string>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/window/renderer/elinux_shader_context.h"
#include "flutter/shell/platform/linux_embedded/window/renderer/elinux_shader_program.h"

namespace flutter {

class ELinuxShader {
 public:
  ELinuxShader() = default;
  ~ELinuxShader() = default;

  void LoadProgram(std::string vertex_code, std::string fragment_code);
  void Bind();
  void Unbind();
  GLuint Program() const { return program_->Program(); }

 private:
  std::unique_ptr<ELinuxShaderProgram> program_;
  std::unique_ptr<ELinuxShaderContext> vertex_;
  std::unique_ptr<ELinuxShaderContext> fragment_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_RENDERER_ELINUX_SHADER_H_
