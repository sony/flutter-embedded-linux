// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ENVIRONMENT_EGL_DRM_EGLSTREAM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ENVIRONMENT_EGL_DRM_EGLSTREAM_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <cstring>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/environment_egl.h"

namespace flutter {

class EnvironmentEglDrmEglstream : public EnvironmentEgl<void> {
 public:
  EnvironmentEglDrmEglstream(int drm_device) : EnvironmentEgl<void>() {
    if (!SetEglExtensionFunctionPointers()) {
      LINUXES_LOG(ERROR) << "Failed to set extension function pointers";
      return;
    }
    auto device = GetEglDevice();
    if (device == EGL_NO_DEVICE_EXT) {
      LINUXES_LOG(ERROR) << "Couldn't find EGL device";
      return;
    }

    EGLint attribs[] = {EGL_DRM_MASTER_FD_EXT, drm_device, EGL_NONE};
    this->display_ =
        eglGetPlatformDisplayEXT_(EGL_PLATFORM_DEVICE_EXT, device, attribs);
    if (this->display_ == EGL_NO_DISPLAY) {
      LINUXES_LOG(ERROR) << "Failed to get the EGL display";
      return;
    }

    if (InitializeEgl()) {
      valid_ = true;
    }
  }

  ~EnvironmentEglDrmEglstream() = default;

 private:
  bool SetEglExtensionFunctionPointers() {
    eglQueryDevicesEXT_ = reinterpret_cast<PFNEGLQUERYDEVICESEXTPROC>(
        eglGetProcAddress("eglQueryDevicesEXT"));
    eglQueryDeviceStringEXT_ = reinterpret_cast<PFNEGLQUERYDEVICESTRINGEXTPROC>(
        eglGetProcAddress("eglQueryDeviceStringEXT"));
    eglGetPlatformDisplayEXT_ =
        reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
            eglGetProcAddress("eglGetPlatformDisplayEXT"));

    return eglQueryDevicesEXT_ && eglQueryDeviceStringEXT_ &&
           eglGetPlatformDisplayEXT_;
  }

  EGLDeviceEXT GetEglDevice() {
    EGLint num_devices;
    if (eglQueryDevicesEXT_(0, nullptr, &num_devices) != EGL_TRUE) {
      LINUXES_LOG(ERROR) << "Failed to query EGL devices";
      return EGL_NO_DEVICE_EXT;
    }
    if (num_devices < 1) {
      LINUXES_LOG(ERROR) << "No EGL devices found";
      return EGL_NO_DEVICE_EXT;
    }

    auto devices = static_cast<EGLDeviceEXT *>(
        std::calloc(num_devices, sizeof(EGLDeviceEXT)));
    if (!devices) {
      LINUXES_LOG(ERROR) << "Failed to allocate memory";
      return EGL_NO_DEVICE_EXT;
    }

    EGLDeviceEXT device = EGL_NO_DEVICE_EXT;
    constexpr char kEglExtensionDeviceDrm[] = "EGL_EXT_device_drm";
    if (eglQueryDevicesEXT_(num_devices, devices, &num_devices) != EGL_TRUE) {
      LINUXES_LOG(ERROR) << "Failed to query EGL devices";
    }
    for (int i = 0; i < num_devices; i++) {
      auto extensions = eglQueryDeviceStringEXT_(devices[i], EGL_EXTENSIONS);
      if (ExtensionIsSupported(extensions, kEglExtensionDeviceDrm)) {
        device = devices[i];
        break;
      }
    }
    free(devices);
    return device;
  }

  bool ExtensionIsSupported(const char *device_extensions,
                            const char *extension) {
    if (device_extensions && extension) {
      auto current = device_extensions;
      auto end = device_extensions + std::strlen(device_extensions);
      auto length = std::strlen(extension);
      while (current < end) {
        auto current_length = std::strcspn(current, " ");
        if ((length == current_length) &&
            (std::strncmp(extension, current, length) == 0)) {
          return true;
        }
        current += (current_length + 1);
      }
    }
    return false;
  }

  PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT_;
  PFNEGLQUERYDEVICESTRINGEXTPROC eglQueryDeviceStringEXT_;
  PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_ENVIRONMENT_EGL_DRM_EGLSTREAM_H_