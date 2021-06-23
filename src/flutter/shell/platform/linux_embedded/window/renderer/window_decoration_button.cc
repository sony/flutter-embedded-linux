// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/window/renderer/window_decoration_button.h"

#ifdef USE_GLES3
#include <GLES3/gl32.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif
#include <EGL/egl.h>

#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

namespace {
enum { ATTRIB_VERTEC = 0, ATTRIB_COLOR, NUM_ATTRIBUTES };

struct GlProcs {
  PFNGLENABLEPROC glEnable;
  PFNGLCLEARCOLORPROC glClearColor;
  PFNGLCLEARPROC glClear;
  PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
  PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
  PFNGLDRAWARRAYSPROC glDrawArrays;
  PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
  PFNGLLINEWIDTHPROC glLineWidth;
  PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
  PFNGLUSEPROGRAMPROC glUseProgram;
  bool valid;
};

static const GlProcs& GlProcs() {
  static struct GlProcs procs = {};
  static bool initialized = false;
  if (!initialized) {
    procs.glEnable =
        reinterpret_cast<PFNGLENABLEPROC>(eglGetProcAddress("glEnable"));
    procs.glClearColor = reinterpret_cast<PFNGLCLEARCOLORPROC>(
        eglGetProcAddress("glClearColor"));
    procs.glClear =
        reinterpret_cast<PFNGLCLEARPROC>(eglGetProcAddress("glClear"));
    procs.glEnableVertexAttribArray =
        reinterpret_cast<PFNGLENABLEVERTEXATTRIBARRAYPROC>(
            eglGetProcAddress("glEnableVertexAttribArray"));
    procs.glLineWidth =
        reinterpret_cast<PFNGLLINEWIDTHPROC>(eglGetProcAddress("glLineWidth"));
    procs.glVertexAttribPointer =
        reinterpret_cast<PFNGLVERTEXATTRIBPOINTERPROC>(
            eglGetProcAddress("glVertexAttribPointer"));
    procs.glDrawArrays = reinterpret_cast<PFNGLDRAWARRAYSPROC>(
        eglGetProcAddress("glDrawArrays"));
    procs.glDisableVertexAttribArray =
        reinterpret_cast<PFNGLDISABLEVERTEXATTRIBARRAYPROC>(
            eglGetProcAddress("glDisableVertexAttribArray"));
    procs.glBindAttribLocation = reinterpret_cast<PFNGLBINDATTRIBLOCATIONPROC>(
        eglGetProcAddress("glBindAttribLocation"));
    procs.glUseProgram = reinterpret_cast<PFNGLUSEPROGRAMPROC>(
        eglGetProcAddress("glUseProgram"));
    procs.valid = procs.glEnable && procs.glClearColor && procs.glClear &&
                  procs.glEnableVertexAttribArray && procs.glLineWidth &&
                  procs.glVertexAttribPointer && procs.glDrawArrays &&
                  procs.glDisableVertexAttribArray &&
                  procs.glBindAttribLocation && procs.glUseProgram;
    if (!procs.valid) {
      ELINUX_LOG(ERROR) << "Failed to load GlProcs";
    }
    initialized = true;
  }
  return procs;
}

constexpr char kGlVertexShader[] =
    "attribute vec4 Position;            \n"
    "attribute vec4 SourceColor;         \n"
    "varying vec4 DestinationColor;      \n"
    "void main() {                       \n"
    "  gl_Position = Position;           \n"
    "  DestinationColor = SourceColor;   \n"
    "}                                   \n";

constexpr char kGlFragmentShader[] =
    "varying lowp vec4 DestinationColor; \n"
    "void main() {                       \n"
    "  gl_FragColor = DestinationColor;  \n"
    "}                                   \n";
}  // namespace

WindowDecorationButton::WindowDecorationButton(
    DecorationType decoration_type,
    std::unique_ptr<NativeWindowWaylandDecoration> native_window,
    std::unique_ptr<SurfaceDecoration> render_surface)
    : shader_(nullptr) {
  decoration_type_ = decoration_type;
  native_window_ = std::move(native_window);
  render_surface_ = std::move(render_surface);
  render_surface_->SetNativeWindow(native_window_.get());
  render_surface_->Resize(native_window_->Width(), native_window_->Height());
}

WindowDecorationButton::~WindowDecorationButton() {
  render_surface_ = nullptr;
  native_window_ = nullptr;
  shader_ = nullptr;
}

void WindowDecorationButton::Draw() {
  const auto& gl = GlProcs();
  if (!gl.valid) {
    return;
  }

  render_surface_->GLContextMakeCurrent();
  {
    gl.glClearColor(100 / 255.0f, 100 / 255.0f, 100 / 255.0f, 1.0f);
    gl.glClear(GL_COLOR_BUFFER_BIT);
    {
      if (!shader_) {
        LoadShader();
      }
      shader_->Bind();
      if (decoration_type_ == DecorationType::CLOSE_BUTTON) {
        GLfloat vertices[] = {
            -0.9f, -0.9f, 0.9f, 0.9f, 0.9f, -0.9f, -0.9f, 0.9f,
        };

        GLfloat colors[] = {
            1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        };

        gl.glEnableVertexAttribArray(ATTRIB_VERTEC);
        gl.glEnableVertexAttribArray(ATTRIB_COLOR);

        gl.glLineWidth(2);
        gl.glVertexAttribPointer(ATTRIB_VERTEC, 2, GL_FLOAT, GL_FALSE, 0,
                                 vertices);
        gl.glVertexAttribPointer(ATTRIB_COLOR, 4, GL_FLOAT, GL_FALSE, 0,
                                 colors);
        gl.glDrawArrays(GL_LINES, 0, 4);

        gl.glDisableVertexAttribArray(ATTRIB_VERTEC);
        gl.glDisableVertexAttribArray(ATTRIB_COLOR);
      } else if (decoration_type_ == DecorationType::MAXIMISE_BUTTON) {
      } else {
      }
      shader_->Unbind();
    }
  }
  render_surface_->GLContextPresent(0);
}

void WindowDecorationButton::SetPosition(const int32_t x, const int32_t y) {
  native_window_->SetPosition(x, y);
}

void WindowDecorationButton::Resize(const int32_t width, const int32_t height) {
  render_surface_->Resize(width, height);
}

void WindowDecorationButton::LoadShader() {
  if (shader_) {
    return;
  }

  shader_ = std::make_unique<Shader>();
  shader_->LoadProgram(kGlVertexShader, kGlFragmentShader);

  const auto& gl = GlProcs();
  gl.glBindAttribLocation(shader_->Program(), ATTRIB_VERTEC, "Position");
  gl.glBindAttribLocation(shader_->Program(), ATTRIB_COLOR, "SourceColor");
}

}  // namespace flutter
