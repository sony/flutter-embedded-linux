// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/window/renderer/window_decorations_wayland.h"

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/surface_gl.h"

namespace flutter {

namespace {
constexpr uint kTitleBarHeight = 30;

constexpr uint kButtonWidth = 15;
constexpr uint kButtonHeight = 15;
constexpr uint kButtonMargin = 10;
}  // namespace

WindowDecorationsWayland::WindowDecorationsWayland(
    wl_display* display, wl_compositor* compositor,
    wl_subcompositor* subcompositor, wl_surface* root_surface, int32_t width,
    int32_t height) {
  titlebar_ = std::make_unique<WindowDecorationTitlebar>(
      std::make_unique<NativeWindowWaylandDecoration>(
          compositor, subcompositor, root_surface, width, kTitleBarHeight),
      std::make_unique<SurfaceDecoration>(std::make_unique<ContextEgl>(
          std::make_unique<EnvironmentEgl>(display))));
  titlebar_->SetPosition(0, -kTitleBarHeight);

  button_ = std::make_unique<WindowDecorationButton>(
      WindowDecorationButton::DecorationType::CLOSE_BUTTON,
      std::make_unique<NativeWindowWaylandDecoration>(
          compositor, subcompositor, root_surface, kButtonWidth, kButtonHeight),
      std::make_unique<SurfaceDecoration>(std::make_unique<ContextEgl>(
          std::make_unique<EnvironmentEgl>(display))));
  button_->SetPosition(
      width - kButtonWidth - kButtonMargin,
      -(kButtonHeight + (kTitleBarHeight - kButtonHeight) / 2));
}

WindowDecorationsWayland::~WindowDecorationsWayland() {
  titlebar_ = nullptr;
  button_ = nullptr;
}

void WindowDecorationsWayland::Draw() {
  titlebar_->Draw();
  button_->Draw();
}

void WindowDecorationsWayland::Resize(const int32_t width,
                                      const int32_t height) {
  titlebar_->SetPosition(0, -kTitleBarHeight);
  titlebar_->Resize(width, kTitleBarHeight);

  button_->SetPosition(
      width - kButtonWidth - kButtonMargin,
      -(kButtonHeight + (kTitleBarHeight - kButtonHeight) / 2));
  button_->Resize(kButtonWidth, -kButtonHeight);
}

bool WindowDecorationsWayland::IsMatched(
    wl_surface* surface,
    WindowDecoration::DecorationType decoration_type) const {
  switch (decoration_type) {
    case WindowDecoration::DecorationType::TITLE_BAR:
      return titlebar_->Surface() == surface;
    case WindowDecoration::DecorationType::CLOSE_BUTTON:
      return button_->Surface() == surface;
    case WindowDecoration::DecorationType::MAXIMISE_BUTTON:
    case WindowDecoration::DecorationType::MINIMISE_BUTTON:
    default:
      return false;
      break;
  }
}

}  // namespace flutter
