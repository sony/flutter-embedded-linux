// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_DRM_EGLSTREAM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_DRM_EGLSTREAM_H_

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <string>

#include "flutter/shell/platform/linux_embedded/window/native_window_drm.h"

namespace flutter {

class NativeWindowDrmEglstream : public NativeWindowDrm {
 public:
  NativeWindowDrmEglstream(const char* deviceFilename);
  ~NativeWindowDrmEglstream();

  // |NativeWindowDrm|
  bool ShowCursor(double x, double y) override;

  // |NativeWindowDrm|
  bool UpdateCursor(const std::string& cursor_name, double x,
                    double y) override;

  // |NativeWindowDrm|
  bool DismissCursor() override;

  // |NativeWindowDrm|
  std::unique_ptr<SurfaceGl> CreateRenderSurface() override;

  uint32_t PlaneId() { return drm_plane_id_; }

 private:
  struct DrmProperty {
    const char* name;
    uint64_t value;
  };

  bool ConfigureDisplayAdditional();

  bool SetDrmClientCapabilities();

  uint32_t FindPrimaryPlaneId(drmModePlaneResPtr resources);

  uint64_t GetPropertyValue(uint32_t id, uint32_t type, const char* prop_name);

  bool AssignAtomicRequest(drmModeAtomicReqPtr atomic);

  template <size_t N>
  bool AssignAtomicPropertyValue(
      drmModeAtomicReqPtr atomic, uint32_t id, uint32_t type,
      NativeWindowDrmEglstream::DrmProperty (&table)[N]);

  uint32_t drm_plane_id_;
  uint32_t drm_property_blob_ = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_DRM_EGLSTREAM_H_
