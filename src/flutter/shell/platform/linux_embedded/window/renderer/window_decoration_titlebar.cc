// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/window/renderer/window_decoration_titlebar.h"

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
struct GlProcs {
  PFNGLCLEARCOLORPROC glClearColor;
  PFNGLCLEARPROC glClear;
  bool valid;
};

static const GlProcs& GlProcs() {
  static struct GlProcs procs = {};
  static bool initialized = false;
  if (!initialized) {
    procs.glClearColor = reinterpret_cast<PFNGLCLEARCOLORPROC>(
        eglGetProcAddress("glClearColor"));
    procs.glClear =
        reinterpret_cast<PFNGLCLEARPROC>(eglGetProcAddress("glClear"));
    procs.valid = procs.glClearColor && procs.glClear;
    if (!procs.valid) {
      ELINUX_LOG(ERROR) << "Failed to load GlProcs";
    }
    initialized = true;
  }
  return procs;
}

}  // namespace

WindowDecorationTitlebar::WindowDecorationTitlebar(
    std::unique_ptr<NativeWindowWaylandDecoration> native_window,
    std::unique_ptr<SurfaceDecoration> render_surface) {
  decoration_type_ = DecorationType::TITLE_BAR;
  native_window_ = std::move(native_window);
  render_surface_ = std::move(render_surface);
  render_surface_->SetNativeWindow(native_window_.get());
  render_surface_->Resize(native_window_->Width(), native_window_->Height());
}

WindowDecorationTitlebar::~WindowDecorationTitlebar() {
  render_surface_ = nullptr;
  native_window_ = nullptr;
}

void WindowDecorationTitlebar::Draw() {
  const auto& gl = GlProcs();
  if (!gl.valid) {
    return;
  }

  render_surface_->GLContextMakeCurrent();
  {
    gl.glClearColor(51 / 255.0f, 51 / 255.0f, 51 / 255.0f, 1.0f);
    gl.glClear(GL_COLOR_BUFFER_BIT);
  }
  render_surface_->GLContextPresent(0);
}

void WindowDecorationTitlebar::SetPosition(const int32_t x, const int32_t y) {
  native_window_->SetPosition(x, y);
}

void WindowDecorationTitlebar::Resize(const int32_t width,
                                      const int32_t height) {
  render_surface_->Resize(width, height);
}

}  // namespace flutter
