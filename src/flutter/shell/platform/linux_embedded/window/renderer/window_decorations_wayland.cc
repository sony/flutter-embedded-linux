// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/window/renderer/window_decorations_wayland.h"

#include "flutter/shell/platform/linux_embedded/surface/environment_egl.h"
#include "flutter/shell/platform/linux_embedded/surface/surface_gl.h"

namespace flutter {

namespace {
constexpr uint kTitleBarHeightDIP = 30;

constexpr uint kButtonWidthDIP = 15;
constexpr uint kButtonHeightDIP = 15;
constexpr uint kButtonMarginDIP = 10;
}  // namespace

WindowDecorationsWayland::WindowDecorationsWayland(
    wl_display* display,
    wl_compositor* compositor,
    wl_subcompositor* subcompositor,
    wl_surface* root_surface,
    int32_t width_dip,
    int32_t height_dip,
    double pixel_ratio,
    bool enable_impeller,
    bool enable_vsync) {
  constexpr bool sub_egl_display = true;

  // title-bar.
  titlebar_ = std::make_unique<WindowDecorationTitlebar>(
      std::make_unique<NativeWindowWaylandDecoration>(
          compositor, subcompositor, root_surface, width_dip * pixel_ratio,
          kTitleBarHeightDIP * pixel_ratio, enable_vsync),
      std::make_unique<SurfaceDecoration>(std::make_unique<ContextEgl>(
          std::make_unique<EnvironmentEgl>(EGL_PLATFORM_WAYLAND_KHR, display, sub_egl_display),
          enable_impeller)));
  titlebar_->SetPosition(0, -kTitleBarHeightDIP);

  // close button.
  auto type = WindowDecorationButton::DecorationType::CLOSE_BUTTON;
  buttons_.push_back(std::make_unique<WindowDecorationButton>(
      type,
      std::make_unique<NativeWindowWaylandDecoration>(
          compositor, subcompositor, root_surface,
          kButtonWidthDIP * pixel_ratio, kButtonHeightDIP * pixel_ratio,
          enable_vsync),
      std::make_unique<SurfaceDecoration>(std::make_unique<ContextEgl>(
          std::make_unique<EnvironmentEgl>(EGL_PLATFORM_WAYLAND_KHR, display, sub_egl_display),
          enable_impeller))));
  buttons_[type]->SetPosition(
      width_dip * pixel_ratio - kButtonWidthDIP - kButtonMarginDIP,
      -(kButtonHeightDIP + (kTitleBarHeightDIP - kButtonHeightDIP) / 2));

  // maximise button.
  type = WindowDecorationButton::DecorationType::MAXIMISE_BUTTON;
  buttons_.push_back(std::make_unique<WindowDecorationButton>(
      type,
      std::make_unique<NativeWindowWaylandDecoration>(
          compositor, subcompositor, root_surface,
          kButtonWidthDIP * pixel_ratio, kButtonHeightDIP * pixel_ratio,
          enable_vsync),
      std::make_unique<SurfaceDecoration>(std::make_unique<ContextEgl>(
          std::make_unique<EnvironmentEgl>(EGL_PLATFORM_WAYLAND_KHR, display, sub_egl_display),
          enable_impeller))));
  buttons_[type]->SetPosition(
      width_dip * pixel_ratio - kButtonWidthDIP * 2 - kButtonMarginDIP * 2,
      -(kButtonHeightDIP + (kTitleBarHeightDIP - kButtonHeightDIP) / 2));

  // minimise button.
  type = WindowDecorationButton::DecorationType::MINIMISE_BUTTON;
  buttons_.push_back(std::make_unique<WindowDecorationButton>(
      type,
      std::make_unique<NativeWindowWaylandDecoration>(
          compositor, subcompositor, root_surface,
          kButtonWidthDIP * pixel_ratio, kButtonHeightDIP * pixel_ratio,
          enable_vsync),
      std::make_unique<SurfaceDecoration>(std::make_unique<ContextEgl>(
          std::make_unique<EnvironmentEgl>(EGL_PLATFORM_WAYLAND_KHR, display, sub_egl_display),
          enable_impeller))));
  buttons_[type]->SetPosition(
      width_dip * pixel_ratio - kButtonWidthDIP * 3 - kButtonMarginDIP * 3,
      -(kButtonHeightDIP + (kTitleBarHeightDIP - kButtonHeightDIP) / 2));
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

void WindowDecorationsWayland::Resize(const int32_t width_dip,
                                      const int32_t height_dip,
                                      double pixel_ratio) {
  titlebar_->SetScaleFactor(pixel_ratio);
  titlebar_->SetPosition(0, -kTitleBarHeightDIP);
  titlebar_->Resize(width_dip * pixel_ratio, kTitleBarHeightDIP * pixel_ratio);

  for (auto i = 0; i < buttons_.size(); i++) {
    buttons_[i]->SetScaleFactor(pixel_ratio);
    buttons_[i]->SetPosition(
        width_dip * pixel_ratio - kButtonWidthDIP * (i + 1) -
            kButtonMarginDIP * (i + 1),
        -(kButtonHeightDIP + (kTitleBarHeightDIP - kButtonHeightDIP) / 2));
    buttons_[i]->Resize(kButtonWidthDIP * pixel_ratio,
                        kButtonHeightDIP * pixel_ratio);
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
  return kTitleBarHeightDIP;
}

}  // namespace flutter
