// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/surface/environment_egl_drm_eglstream.h"

#include <cstring>

#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

EnvironmentEglDrmEglstream::EnvironmentEglDrmEglstream() : EnvironmentEgl() {
  if (!SetEglExtensionFunctionPointers()) {
    LINUXES_LOG(ERROR) << "Failed to set extension function pointers";
    return;
  }
  auto device = GetEglDevice();
  if (device == EGL_NO_DEVICE_EXT) {
    LINUXES_LOG(ERROR) << "Couldn't find EGL device";
    return;
  }

  display_ = eglGetPlatformDisplayEXT_(EGL_PLATFORM_DEVICE_EXT, device, NULL);
  if (display_ == EGL_NO_DISPLAY) {
    LINUXES_LOG(ERROR) << "Failed to get the EGL display";
    return;
  }

  valid_ = InitializeEgl();
}

bool EnvironmentEglDrmEglstream::SetEglExtensionFunctionPointers() {
  eglQueryDevicesEXT_ = reinterpret_cast<PFNEGLQUERYDEVICESEXTPROC>(
      eglGetProcAddress("eglQueryDevicesEXT"));
  eglQueryDeviceStringEXT_ = reinterpret_cast<PFNEGLQUERYDEVICESTRINGEXTPROC>(
      eglGetProcAddress("eglQueryDeviceStringEXT"));
  eglGetPlatformDisplayEXT_ = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
      eglGetProcAddress("eglGetPlatformDisplayEXT"));

  return eglQueryDevicesEXT_ && eglQueryDeviceStringEXT_ &&
         eglGetPlatformDisplayEXT_;
}

EGLDeviceEXT EnvironmentEglDrmEglstream::GetEglDevice() {
  EGLint num_devices;
  if (eglQueryDevicesEXT_(0, NULL, &num_devices) != EGL_TRUE) {
    LINUXES_LOG(ERROR) << "Failed to query EGL devices";
    return EGL_NO_DEVICE_EXT;
  }
  if (num_devices < 1) {
    LINUXES_LOG(ERROR) << "No EGL devices found";
    return EGL_NO_DEVICE_EXT;
  }

  auto devices = static_cast<EGLDeviceEXT*>(
      std::calloc(num_devices, sizeof(EGLDeviceEXT)));
  if (!devices) {
    LINUXES_LOG(ERROR) << "Failed to allocate memory";
    return EGL_NO_DEVICE_EXT;
  }

  if (eglQueryDevicesEXT_(num_devices, devices, &num_devices) != EGL_TRUE) {
    LINUXES_LOG(ERROR) << "Failed to query EGL devices";
    std::free(devices);
    return EGL_NO_DEVICE_EXT;
  }

  EGLDeviceEXT device = EGL_NO_DEVICE_EXT;
  constexpr char kEglExtensionDeviceDrm[] = "EGL_EXT_device_drm";
  for (int i = 0; i < num_devices; i++) {
    auto extensions = eglQueryDeviceStringEXT_(devices[i], EGL_EXTENSIONS);
    if (extensions && (std::string(extensions).find(kEglExtensionDeviceDrm) !=
                       std::string::npos)) {
      device = devices[i];
      break;
    }
  }
  std::free(devices);
  return device;
}

}  // namespace flutter
