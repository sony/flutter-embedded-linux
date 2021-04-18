// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_DRM_GBM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_DRM_GBM_H_

#include <gbm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <string>

#include "flutter/shell/platform/linux_embedded/window/native_window_drm.h"

namespace flutter {

class NativeWindowDrmGbm : public NativeWindowDrm {
 public:
  NativeWindowDrmGbm(const char* deviceFilename);
  ~NativeWindowDrmGbm();

  // |NativeWindowDrm|
  bool ShowCursor(double x, double y) override;

  // |NativeWindowDrm|
  bool UpdateCursor(const std::string& cursor_name, double x,
                    double y) override;

  // |NativeWindowDrm|
  bool DismissCursor() override;

  // |NativeWindowDrm|
  std::unique_ptr<SurfaceGl> CreateRenderSurface() override;

  // |NativeWindow|
  void SwapBuffers() override;

 private:
  bool CreateCursorBuffer(const std::string& cursor_name);

  gbm_bo* gbm_previous_bo_ = nullptr;
  uint32_t gbm_previous_fb_;
  gbm_device* gbm_device_ = nullptr;
  gbm_bo* gbm_cursor_bo_ = nullptr;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_DRM_GBM_H_
