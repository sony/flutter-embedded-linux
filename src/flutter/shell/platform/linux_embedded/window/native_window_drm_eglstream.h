// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_DRM_EGLSTREAM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_DRM_EGLSTREAM_H_

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <string>

#include "flutter/shell/platform/linux_embedded/surface/context_egl_drm_eglstream.h"
#include "flutter/shell/platform/linux_embedded/surface/linuxes_surface_gl_drm.h"
#include "flutter/shell/platform/linux_embedded/window/native_window_drm.h"

namespace flutter {

class NativeWindowDrmEglstream
    : public NativeWindowDrm<SurfaceGlDrm<ContextEglDrmEglstream>> {
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
  std::unique_ptr<SurfaceGlDrm<ContextEglDrmEglstream>> CreateRenderSurface()
      override;

  uint32_t PlaneId() { return drm_plane_id_; }

 private:
  struct DrmPropertyIds {
    struct {
      uint32_t mode_id;
      uint32_t active;
    } crtc;

    struct {
      uint32_t src_x;
      uint32_t src_y;
      uint32_t src_w;
      uint32_t src_h;
      uint32_t crtc_x;
      uint32_t crtc_y;
      uint32_t crtc_w;
      uint32_t crtc_h;
      uint32_t fb_id;
      uint32_t crtc_id;
    } plane;

    struct {
      uint32_t crtc_id;
    } connector;
  };

  struct DrmPropertyAddress {
    const char* name;
    uint32_t* ptr;
  };

  bool ConfigureDisplayAdditional();

  bool SetDrmClientCapabilities();

  uint32_t FindPrimaryPlaneId(drmModePlaneResPtr resources);

  uint64_t GetPropertyValue(uint32_t id, uint32_t type, const char* prop_name);

  bool AssignAtomicRequest(drmModeAtomicReqPtr atomic);

  void GetPropertyIds(DrmPropertyIds& property_ids);

  void GetPropertyAddress(uint32_t id, uint32_t type, DrmPropertyAddress* table,
                          size_t length);

  bool CreateFb();

  uint32_t drm_plane_id_;
  uint32_t drm_property_blob_ = 0;
  uint32_t drm_fb_ = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_NATIVE_WINDOW_DRM_EGLSTREAM_H_