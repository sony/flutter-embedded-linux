// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/window/renderer/window_decorations_wayland.h"

#include "flutter/shell/platform/linux_embedded/surface/environment_egl.h"
#include "flutter/shell/platform/linux_embedded/surface/surface_gl.h"

namespace flutter {

namespace {
constexpr uint kTitleBarHeight = 30;

constexpr uint kButtonWidth = 15;
constexpr uint kButtonHeight = 15;
constexpr uint kButtonMargin = 10;
}  // namespace

WindowDecorationsWayland::WindowDecorationsWayland(
    wl_display* display,
    wl_compositor* compositor,
    wl_subcompositor* subcompositor,
    wl_surface* root_surface,
    int32_t width,
    int32_t height) {
  constexpr bool sub_egl_display = true;

  // title-bar.
  titlebar_ = std::make_unique<WindowDecorationTitlebar>(
      std::make_unique<NativeWindowWaylandDecoration>(
          compositor, subcompositor, root_surface, width, kTitleBarHeight),
      std::make_unique<SurfaceDecoration>(std::make_unique<ContextEgl>(
          std::make_unique<EnvironmentEgl>(display, sub_egl_display))));
  titlebar_->SetPosition(0, -kTitleBarHeight);

  // close button.
  auto type = WindowDecorationButton::DecorationType::CLOSE_BUTTON;
  buttons_.push_back(std::make_unique<WindowDecorationButton>(
      type,
      std::make_unique<NativeWindowWaylandDecoration>(
          compositor, subcompositor, root_surface, kButtonWidth, kButtonHeight),
      std::make_unique<SurfaceDecoration>(std::make_unique<ContextEgl>(
          std::make_unique<EnvironmentEgl>(display, sub_egl_display)))));
  buttons_[type]->SetPosition(
      width - kButtonWidth - kButtonMargin,
      -(kButtonHeight + (kTitleBarHeight - kButtonHeight) / 2));

  // maximise button.
  type = WindowDecorationButton::DecorationType::MAXIMISE_BUTTON;
  buttons_.push_back(std::make_unique<WindowDecorationButton>(
      type,
      std::make_unique<NativeWindowWaylandDecoration>(
          compositor, subcompositor, root_surface, kButtonWidth, kButtonHeight),
      std::make_unique<SurfaceDecoration>(std::make_unique<ContextEgl>(
          std::make_unique<EnvironmentEgl>(display, sub_egl_display)))));
  buttons_[type]->SetPosition(
      width - kButtonWidth * 2 - kButtonMargin * 2,
      -(kButtonHeight + (kTitleBarHeight - kButtonHeight) / 2));

  // minimise button.
  type = WindowDecorationButton::DecorationType::MINIMISE_BUTTON;
  buttons_.push_back(std::make_unique<WindowDecorationButton>(
      type,
      std::make_unique<NativeWindowWaylandDecoration>(
          compositor, subcompositor, root_surface, kButtonWidth, kButtonHeight),
      std::make_unique<SurfaceDecoration>(std::make_unique<ContextEgl>(
          std::make_unique<EnvironmentEgl>(display, sub_egl_display)))));
  buttons_[type]->SetPosition(
      width - kButtonWidth * 3 - kButtonMargin * 3,
      -(kButtonHeight + (kTitleBarHeight - kButtonHeight) / 2));
}

WindowDecorationsWayland::~WindowDecorationsWayland() {
  DestroyContext();
  titlebar_ = nullptr;
  for (auto& b : buttons_) {
    buttons_.pop_back();
  }
}

void WindowDecorationsWayland::Draw() {
  titlebar_->Draw();
  for (auto& b : buttons_) {
    b->Draw();
  }
}

void WindowDecorationsWayland::Resize(const int32_t width,
                                      const int32_t height) {
  titlebar_->SetPosition(0, -kTitleBarHeight);
  titlebar_->Resize(width, kTitleBarHeight);

  for (auto i = 0; i < buttons_.size(); i++) {
    buttons_[i]->SetPosition(
        width - kButtonWidth * (i + 1) - kButtonMargin * (i + 1),
        -(kButtonHeight + (kTitleBarHeight - kButtonHeight) / 2));
    buttons_[i]->Resize(kButtonWidth, kButtonHeight);
  }
}

bool WindowDecorationsWayland::IsMatched(
    wl_surface* surface,
    WindowDecoration::DecorationType decoration_type) const {
  switch (decoration_type) {
    case WindowDecoration::DecorationType::TITLE_BAR:
      return titlebar_->Surface() == surface;
    default:
      return buttons_[decoration_type]->Surface() == surface;
  }
}

void WindowDecorationsWayland::DestroyContext() {
  titlebar_->DestroyContext();
  for (auto& b : buttons_) {
    b->DestroyContext();
  }
}

int32_t WindowDecorationsWayland::Height() const {
  return kTitleBarHeight;
}

}  // namespace flutter
