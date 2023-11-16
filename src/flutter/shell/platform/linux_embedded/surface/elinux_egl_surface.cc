// Copyright 2023 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/surface/elinux_egl_surface.h"

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/egl_utils.h"

namespace flutter {

constexpr size_t kInitialWindowWidthPx = 1280;
constexpr size_t kInitialWindowHeightPx = 720;

// Maximum damage history - for triple buffering we need to store damage for
// last two frames; Some Android devices (Pixel 4) use quad buffering.
constexpr const int kMaxHistorySize = 10;

ELinuxEGLSurface::ELinuxEGLSurface(EGLSurface surface,
                                   EGLDisplay display,
                                   EGLContext context,
                                   bool vsync_enabled)
    : surface_(surface),
      display_(display),
      context_(context),
      vsync_enabled_(vsync_enabled),
      width_px_(kInitialWindowWidthPx),
      height_px_(kInitialWindowHeightPx) {
  const char* extensions = eglQueryString(display_, EGL_EXTENSIONS);

  if (has_egl_extension(extensions, "EGL_KHR_partial_update")) {
    eglSetDamageRegionKHR_ = reinterpret_cast<PFNEGLSETDAMAGEREGIONKHRPROC>(
        eglGetProcAddress("eglSetDamageRegionKHR"));
  }

  if (has_egl_extension(extensions, "EGL_EXT_swap_buffers_with_damage")) {
    eglSwapBuffersWithDamageEXT_ =
        reinterpret_cast<PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC>(
            eglGetProcAddress("eglSwapBuffersWithDamageEXT"));
  } else if (has_egl_extension(extensions,
                               "EGL_KHR_swap_buffers_with_damage")) {
    eglSwapBuffersWithDamageEXT_ =
        reinterpret_cast<PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC>(
            eglGetProcAddress("eglSwapBuffersWithDamageKHR"));
  } else {
    // do nothing.
  }
};

ELinuxEGLSurface::~ELinuxEGLSurface() {
  if (surface_ != EGL_NO_SURFACE) {
    if (eglDestroySurface(display_, surface_) != EGL_TRUE) {
      ELINUX_LOG(ERROR) << "Failed to destory surface: "
                        << get_egl_error_cause();
    }
    surface_ = EGL_NO_SURFACE;
  }
}

bool ELinuxEGLSurface::IsValid() const {
  return surface_ != EGL_NO_SURFACE;
}

void ELinuxEGLSurface::SurfaceResize(const size_t width_px,
                                     const size_t height_px) {
  width_px_ = width_px;
  height_px_ = height_px;
}

bool ELinuxEGLSurface::MakeCurrent() const {
  if (eglMakeCurrent(display_, surface_, surface_, context_) != EGL_TRUE) {
    ELINUX_LOG(ERROR) << "Failed to make the EGL context current: "
                      << get_egl_error_cause();
    return false;
  }

  // Non-blocking when swappipping buffers on Wayland.
  // OpenGL swap intervals can be used to prevent screen tearing.
  // If enabled, the raster thread blocks until the v-blank.
  // This is unnecessary if DWM composition is enabled.
  // See: https://www.khronos.org/opengl/wiki/Swap_Interval
  // See: https://learn.microsoft.com/windows/win32/dwm/composition-ovw
  //
  // However, we might encounter rendering problems on some Wayland compositors
  // (e.g. weston 9.0).
  // See also:
  //   - https://github.com/sony/flutter-embedded-linux/issues/230
  //   - https://github.com/sony/flutter-embedded-linux/issues/234
  //   - https://github.com/sony/flutter-embedded-linux/issues/220
  if (!vsync_enabled_) {
    if (eglSwapInterval(display_, 0) != EGL_TRUE) {
      ELINUX_LOG(ERROR) << "Failed to eglSwapInterval(Free): "
                        << get_egl_error_cause();
    }
  }

  return true;
}

bool ELinuxEGLSurface::SwapBuffers() const {
  if (eglSwapBuffers(display_, surface_) != EGL_TRUE) {
    ELINUX_LOG(ERROR) << "eglSwapBuffers failed: " << get_egl_error_cause();
    return false;
  }
  return true;
}

// Reference of dirty region management:
// https://github.com/flutter/engine/blob/main/examples/glfw_drm/FlutterEmbedderGLFW.cc

bool ELinuxEGLSurface::SwapBuffers(const FlutterPresentInfo* info) {
  // Free the existing damage that was allocated to this frame.
  if (existing_damage_map_.find(info->fbo_id) != existing_damage_map_.end() &&
      existing_damage_map_[info->fbo_id] != nullptr) {
    free(existing_damage_map_[info->fbo_id]);
    existing_damage_map_[info->fbo_id] = nullptr;
  }

  // Set the buffer damage as the damage region.
  if (eglSetDamageRegionKHR_) {
    auto buffer_rects = RectToInts(info->buffer_damage.damage[0]);
    if (eglSetDamageRegionKHR_(display_, surface_, buffer_rects.data(), 1) !=
        EGL_TRUE) {
      ELINUX_LOG(ERROR) << "eglSetDamageRegionKHR failed: "
                        << get_egl_error_cause();
      return false;
    }
  }

  // Add frame damage to damage history
  damage_history_.push_back(info->frame_damage.damage[0]);
  if (damage_history_.size() > kMaxHistorySize) {
    damage_history_.pop_front();
  }

  if (eglSwapBuffersWithDamageEXT_) {
    auto frame_rects = RectToInts(info->frame_damage.damage[0]);
    if (eglSwapBuffersWithDamageEXT_(display_, surface_, frame_rects.data(),
                                     1) != EGL_TRUE) {
      ELINUX_LOG(ERROR) << "eglSwapBuffersWithDamageEXT failed: "
                        << get_egl_error_cause();
      return false;
    }
  } else {
    // If the required extensions for partial repaint were not provided, do
    // full repaint.
    if (eglSwapBuffers(display_, surface_) != EGL_TRUE) {
      ELINUX_LOG(ERROR) << "eglSwapBuffers failed: " << get_egl_error_cause();
      return false;
    }
  }

  return true;
}

void ELinuxEGLSurface::PopulateExistingDamage(const intptr_t fbo_id,
                                              FlutterDamage* existing_damage) {
  // Given the FBO age, create existing damage region by joining all frame
  // damages since FBO was last used
  EGLint age = 0;
  if (eglQuerySurface(display_, surface_, EGL_BUFFER_AGE_EXT, &age) !=
          EGL_TRUE ||
      age == 0) {
    age = 4;  // Virtually no driver should have a swapchain length > 4.
  }

  existing_damage->num_rects = 1;

  // Allocate the array of rectangles for the existing damage.
  existing_damage_map_[fbo_id] = static_cast<FlutterRect*>(
      malloc(sizeof(FlutterRect) * existing_damage->num_rects));

  existing_damage_map_[fbo_id][0] = FlutterRect{
      0, 0, static_cast<double>(width_px_), static_cast<double>(height_px_)};
  existing_damage->damage = existing_damage_map_[fbo_id];

  if (age > 1) {
    --age;
    // join up to (age - 1) last rects from damage history
    for (auto i = damage_history_.rbegin();
         i != damage_history_.rend() && age > 0; ++i, --age) {
      if (i == damage_history_.rbegin()) {
        if (i != damage_history_.rend()) {
          existing_damage->damage[0] = {i->left, i->top, i->right, i->bottom};
        }
      } else {
        // Auxiliary function to union the damage regions comprised by two
        // FlutterRect element. It saves the result of this join in the rect
        // variable.
        FlutterRect* rect = &(existing_damage->damage[0]);
        const FlutterRect additional_rect = *i;

        rect->left = std::min(rect->left, additional_rect.left);
        rect->top = std::min(rect->top, additional_rect.top);
        rect->right = std::max(rect->right, additional_rect.right);
        rect->bottom = std::max(rect->bottom, additional_rect.bottom);
      }
    }
  }
}

// Auxiliary function used to transform a FlutterRect into the format that is
// expected by the EGL functions (i.e. array of EGLint).
std::array<EGLint, 4> ELinuxEGLSurface::RectToInts(const FlutterRect rect) {
  EGLint height;
  eglQuerySurface(display_, surface_, EGL_HEIGHT, &height);

  std::array<EGLint, 4> res{
      static_cast<int>(rect.left),
      height - static_cast<int>(rect.bottom),
      static_cast<int>(rect.right) - static_cast<int>(rect.left),
      static_cast<int>(rect.bottom) - static_cast<int>(rect.top),
  };
  return res;
}

}  // namespace flutter
