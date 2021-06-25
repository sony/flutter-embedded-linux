// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifdef USE_GLES3
#include <GLES3/gl32.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif
#include <EGL/egl.h>

#include <string>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/window/renderer/shader_program.h"

namespace flutter {

namespace {

struct GlProcs {
  PFNGLCREATEPROGRAMPROC glCreateProgram;
  PFNGLATTACHSHADERPROC glAttachShader;
  PFNGLLINKPROGRAMPROC glLinkProgram;
  PFNGLGETPROGRAMIVPROC glGetProgramiv;
  PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
  PFNGLDELETEPROGRAMPROC glDeleteProgram;
  bool valid;
};

static const GlProcs& GlProcs() {
  static struct GlProcs procs = {};
  static bool initialized = false;
  if (!initialized) {
    procs.glCreateProgram = reinterpret_cast<PFNGLCREATEPROGRAMPROC>(
        eglGetProcAddress("glCreateProgram"));
    procs.glAttachShader = reinterpret_cast<PFNGLATTACHSHADERPROC>(
        eglGetProcAddress("glAttachShader"));
    procs.glLinkProgram = reinterpret_cast<PFNGLLINKPROGRAMPROC>(
        eglGetProcAddress("glLinkProgram"));
    procs.glGetProgramiv = reinterpret_cast<PFNGLGETPROGRAMIVPROC>(
        eglGetProcAddress("glGetProgramiv"));
    procs.glGetProgramInfoLog = reinterpret_cast<PFNGLGETPROGRAMINFOLOGPROC>(
        eglGetProcAddress("glGetProgramInfoLog"));
    procs.glDeleteProgram = reinterpret_cast<PFNGLDELETEPROGRAMPROC>(
        eglGetProcAddress("glDeleteProgram"));
    procs.valid = procs.glCreateProgram && procs.glAttachShader &&
                  procs.glLinkProgram && procs.glGetProgramiv &&
                  procs.glGetProgramInfoLog && procs.glDeleteProgram;
    if (!procs.valid) {
      ELINUX_LOG(ERROR) << "Failed to load GlProcs";
    }
    initialized = true;
  }
  return procs;
}

}  // namespace

ShaderProgram::ShaderProgram(std::unique_ptr<ShaderContext> vertex_shader,
                             std::unique_ptr<ShaderContext> fragment_shader)
    : vertex_shader_(std::move(vertex_shader)),
      fragment_shader_(std::move(fragment_shader)),
      program_(0) {
  const auto& gl = GlProcs();
  if (!gl.valid) {
    return;
  }

  program_ = gl.glCreateProgram();
  if (!program_) {
    ELINUX_LOG(ERROR) << "Failed to create a shader program";
    return;
  }

  gl.glAttachShader(program_, vertex_shader_->Shader());
  gl.glAttachShader(program_, fragment_shader_->Shader());
  gl.glLinkProgram(program_);

  GLint success;
  gl.glGetProgramiv(program_, GL_LINK_STATUS, &success);
  if (success != GL_TRUE) {
    GLchar buf[1024];

    gl.glGetProgramInfoLog(program_, sizeof(buf), nullptr, buf);
    ELINUX_LOG(ERROR) << "Couldn't link the program: " << buf;

    gl.glDeleteProgram(program_);
    program_ = 0;
  }
}

ShaderProgram::~ShaderProgram() {
  const auto& gl = GlProcs();
  if (!gl.valid) {
    return;
  }
  gl.glDeleteProgram(program_);
}

}  // namespace flutter
