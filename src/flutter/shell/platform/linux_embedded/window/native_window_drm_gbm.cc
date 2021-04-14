// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/window/native_window_drm_gbm.h"

#include <unistd.h>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/cursor_data.h"

namespace flutter {

NativeWindowDrmGbm::NativeWindowDrmGbm(const char* deviceFilename)
    : NativeWindowDrm(deviceFilename) {
  if (!valid_) {
    return;
  }

  if (!drmIsMaster(drm_device_)) {
    LINUXES_LOG(ERROR)
        << "Couldn't become the DRM master. Please confirm if another display "
           "backend such as X11 and Wayland is not running.";
    valid_ = false;
    return;
  }

  gbm_device_ = gbm_create_device(drm_device_);
  if (!gbm_device_) {
    LINUXES_LOG(ERROR) << "Couldn't create the GBM device.";
    valid_ = false;
    return;
  }

  window_ = gbm_surface_create(gbm_device_, drm_mode_info_.hdisplay,
                               drm_mode_info_.vdisplay, GBM_BO_FORMAT_ARGB8888,
                               GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
  if (!window_) {
    LINUXES_LOG(ERROR) << "Failed to create the gbm surface.";
    valid_ = false;
    return;
  }
}

NativeWindowDrmGbm::~NativeWindowDrmGbm() {
  if (drm_device_ == -1) {
    return;
  }

  if (gbm_cursor_bo_) {
    gbm_bo_destroy(gbm_cursor_bo_);
    gbm_cursor_bo_ = nullptr;
  }

  if (drm_crtc_) {
    drmModeSetCrtc(drm_device_, drm_crtc_->crtc_id, drm_crtc_->buffer_id,
                   drm_crtc_->x, drm_crtc_->y, &drm_connector_id_, 1,
                   &drm_crtc_->mode);
    drmModeFreeCrtc(drm_crtc_);
  }

  if (gbm_previous_bo_) {
    drmModeRmFB(drm_device_, gbm_previous_fb_);
    gbm_surface_release_buffer(static_cast<gbm_surface*>(window_),
                               gbm_previous_bo_);
    gbm_surface_destroy(static_cast<gbm_surface*>(window_));
    window_ = nullptr;
  }

  if (gbm_device_) {
    gbm_device_destroy(gbm_device_);
  }

  close(drm_device_);
}

bool NativeWindowDrmGbm::ShowCursor(double x, double y) {
  if (!gbm_cursor_bo_ && !CreateCursorBuffer(cursor_name_)) {
    return false;
  }

  MoveCursor(x, y);
  auto result = drmModeSetCursor(drm_device_, drm_crtc_->crtc_id,
                                 gbm_bo_get_handle(gbm_cursor_bo_).u32,
                                 kCursorBufferWidth, kCursorBufferHeight);
  if (result != 0) {
    LINUXES_LOG(ERROR) << "Failed to set cursor buffer. (" << result << ")";
    return false;
  }
  return true;
}

bool NativeWindowDrmGbm::UpdateCursor(const std::string& cursor_name, double x,
                                      double y) {
  if (cursor_name.compare(cursor_name_) == 0) {
    return true;
  }
  cursor_name_ = cursor_name;

  if (cursor_name.compare(kCursorNameNone) == 0) {
    return DismissCursor();
  }

  if (!CreateCursorBuffer(cursor_name)) {
    return false;
  }

  MoveCursor(x, y);
  auto result = drmModeSetCursor(drm_device_, drm_crtc_->crtc_id,
                                 gbm_bo_get_handle(gbm_cursor_bo_).u32,
                                 kCursorBufferWidth, kCursorBufferHeight);
  if (result != 0) {
    LINUXES_LOG(ERROR) << "Failed to set cursor buffer. (" << result << ")";
    return false;
  }
  return true;
}

bool NativeWindowDrmGbm::DismissCursor() {
  auto result = drmModeSetCursor(drm_device_, drm_crtc_->crtc_id, 0, 0, 0);
  if (result != 0) {
    LINUXES_LOG(ERROR) << "Failed to set cursor buffer. (" << result << ")";
    return false;
  }
  return true;
}

std::unique_ptr<SurfaceGlDrm<ContextEgl>>
NativeWindowDrmGbm::CreateRenderSurface() {
  return std::make_unique<SurfaceGlDrm<ContextEgl>>(
      std::make_unique<ContextEgl>(
          std::make_unique<EnvironmentEgl>(gbm_device_)));
}

void NativeWindowDrmGbm::SwapBuffer() {
  auto* bo = gbm_surface_lock_front_buffer(static_cast<gbm_surface*>(window_));
  auto width = gbm_bo_get_width(bo);
  auto height = gbm_bo_get_height(bo);
  auto handle = gbm_bo_get_handle(bo).u32;
  auto stride = gbm_bo_get_stride(bo);
  uint32_t fb;
  int result =
      drmModeAddFB(drm_device_, width, height, 24, 32, stride, handle, &fb);
  if (result != 0) {
    LINUXES_LOG(ERROR) << "Failed to add a framebuffer. (" << result << ")";
  }
  result = drmModeSetCrtc(drm_device_, drm_crtc_->crtc_id, fb, 0, 0,
                          &drm_connector_id_, 1, &drm_mode_info_);
  if (result != 0) {
    LINUXES_LOG(ERROR) << "Failed to set crct mode. (" << result << ")";
  }

  if (gbm_previous_bo_) {
    drmModeRmFB(drm_device_, gbm_previous_fb_);
    gbm_surface_release_buffer(static_cast<gbm_surface*>(window_),
                               gbm_previous_bo_);
  }
  gbm_previous_bo_ = bo;
  gbm_previous_fb_ = fb;
}

bool NativeWindowDrmGbm::CreateCursorBuffer(const std::string& cursor_name) {
  if (!gbm_cursor_bo_) {
    gbm_cursor_bo_ = gbm_bo_create(gbm_device_, kCursorBufferWidth,
                                   kCursorBufferHeight, GBM_BO_FORMAT_ARGB8888,
                                   GBM_BO_USE_CURSOR | GBM_BO_USE_WRITE);
    if (!gbm_cursor_bo_) {
      LINUXES_LOG(ERROR) << "Failed to create cursor buffer";
      return false;
    }
  }

  auto cursor_data = GetCursorData(cursor_name);
  uint32_t buf[kCursorBufferWidth * kCursorBufferHeight] = {0};
  for (int i = 0; i < kCursorHeight; i++) {
    memcpy(buf + i * kCursorBufferWidth, cursor_data + i * kCursorWidth,
           kCursorWidth * sizeof(uint32_t));
  }

  auto result = gbm_bo_write(gbm_cursor_bo_, buf, sizeof(buf));
  if (result != 0) {
    LINUXES_LOG(ERROR) << "Failed to write cursor data. (" << result << ")";
    return false;
  }
  return true;
}

}  // namespace flutter