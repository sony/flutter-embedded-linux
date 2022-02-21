// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_ELINUX_WINDOW_DRM_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_ELINUX_WINDOW_DRM_H_

#include <fcntl.h>
#include <libinput.h>
#include <linux/input-event-codes.h>
#include <systemd/sd-event.h>
#include <unistd.h>

#include <chrono>
#include <memory>
#include <unordered_map>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/surface_gl.h"
#include "flutter/shell/platform/linux_embedded/window/elinux_window.h"
#include "flutter/shell/platform/linux_embedded/window/native_window_drm.h"
#include "flutter/shell/platform/linux_embedded/window_binding_handler.h"

namespace flutter {

namespace {
constexpr char kFlutterDrmDeviceEnvironmentKey[] = "FLUTTER_DRM_DEVICE";
constexpr char kDrmDeviceDefaultFilename[] = "/dev/dri/card0";
}  // namespace

template <typename T>
class ELinuxWindowDrm : public ELinuxWindow, public WindowBindingHandler {
 public:
  ELinuxWindowDrm(FlutterDesktopViewProperties view_properties)
      : display_valid_(false), is_pending_cursor_add_event_(false) {
    view_properties_ = view_properties;
    SetRotation(view_properties_.view_rotation);

    auto udev = udev_new();
    if (!udev) {
      ELINUX_LOG(ERROR) << "Failed to create udev instance.";
      return;
    }
    libinput_ = libinput_udev_create_context(&kLibinputInterface, NULL, udev);
    if (!libinput_) {
      ELINUX_LOG(ERROR) << "Failed to create libinput instance.";
      udev_unref(udev);
      return;
    }
    udev_unref(udev);

    constexpr char kSeatId[] = "seat0";
    auto ret = libinput_udev_assign_seat(libinput_, kSeatId);
    if (ret != 0) {
      ELINUX_LOG(ERROR) << "Failed to assign udev seat to libinput instance.";
      return;
    }

    ret = sd_event_new(&libinput_event_loop_);
    if (ret < 0) {
      ELINUX_LOG(ERROR) << "Failed to create libinput event loop.";
      return;
    }
    ret =
        sd_event_add_io(libinput_event_loop_, NULL, libinput_get_fd(libinput_),
                        EPOLLIN | EPOLLRDHUP | EPOLLPRI, OnLibinputEvent, this);
    if (ret < 0) {
      ELINUX_LOG(ERROR) << "Failed to listen for user input.";
      libinput_event_loop_ = sd_event_unref(libinput_event_loop_);
      return;
    }
  }

  ~ELinuxWindowDrm() {
    if (udev_drm_event_loop_) {
      sd_event_unref(udev_drm_event_loop_);
    }

    if (udev_monitor_) {
      udev_monitor_unref(udev_monitor_);
    }

    if (libinput_event_loop_) {
      sd_event_unref(libinput_event_loop_);
    }
    libinput_unref(libinput_);
    display_valid_ = false;
  }

  // |ELinuxWindow|
  bool IsValid() const override {
    if (!display_valid_ || !native_window_ || !render_surface_ ||
        !native_window_->IsValid() || !render_surface_->IsValid()) {
      return false;
    }
    return true;
  }

  // |FlutterWindowBindingHandler|
  bool DispatchEvent() override {
    constexpr uint64_t kMaxWaitTime = 0;
    sd_event_run(libinput_event_loop_, kMaxWaitTime);
    sd_event_run(udev_drm_event_loop_, kMaxWaitTime);
    return true;
  }

  // |FlutterWindowBindingHandler|
  bool CreateRenderSurface(int32_t width, int32_t height) override {
    auto device_filename = std::getenv(kFlutterDrmDeviceEnvironmentKey);
    if ((!device_filename) || (device_filename[0] == '\0')) {
      ELINUX_LOG(WARNING) << kFlutterDrmDeviceEnvironmentKey
                          << " is not set, use " << kDrmDeviceDefaultFilename;
      device_filename = const_cast<char*>(kDrmDeviceDefaultFilename);
    }

    if (current_rotation_ == 90 || current_rotation_ == 270) {
      std::swap(width, height);
    }
    native_window_ = std::make_unique<T>(device_filename);
    if (!native_window_->IsValid()) {
      ELINUX_LOG(ERROR) << "Failed to create the native window";
      return false;
    }

    if (!RegisterUdevDrmEventLoop(device_filename)) {
      ELINUX_LOG(ERROR) << "Failed to register udev drm event loop.";
      return false;
    }
    display_valid_ = true;

    render_surface_ = native_window_->CreateRenderSurface();
    if (!render_surface_->SetNativeWindow(native_window_.get())) {
      return false;
    }

    if (view_properties_.view_mode != FlutterDesktopViewMode::kFullscreen) {
      ELINUX_LOG(WARNING)
          << "Normal mode is not supported, use fullscreen mode.";
      view_properties_.view_mode = FlutterDesktopViewMode::kFullscreen;
    }
    view_properties_.width = native_window_->Width();
    view_properties_.height = native_window_->Height();
    ELINUX_LOG(INFO) << "Display output resolution: " << view_properties_.width
                     << "x" << view_properties_.height;

    if (is_pending_cursor_add_event_) {
      native_window_->ShowCursor(pointer_x_, pointer_y_);
      is_pending_cursor_add_event_ = false;
    }

    return true;
  }

  // |FlutterWindowBindingHandler|
  void DestroyRenderSurface() override {
    // destroy the main surface before destroying the client window on DRM.
    render_surface_ = nullptr;
    native_window_ = nullptr;
  }

  // |FlutterWindowBindingHandler|
  void SetView(WindowBindingHandlerDelegate* view) override {
    binding_handler_delegate_ = view;
  }

  // |FlutterWindowBindingHandler|
  ELinuxRenderSurfaceTarget* GetRenderSurfaceTarget() const override {
    return render_surface_.get();
  }

  // |FlutterWindowBindingHandler|
  uint16_t GetRotationDegree() const override { return current_rotation_; }

  // |FlutterWindowBindingHandler|
  double GetDpiScale() override { return current_scale_; }

  // |FlutterWindowBindingHandler|
  PhysicalWindowBounds GetPhysicalWindowBounds() override {
    return {GetCurrentWidth(), GetCurrentHeight()};
  }

  // |FlutterWindowBindingHandler|
  int32_t GetFrameRate() override { return 60000; }

  // |FlutterWindowBindingHandler|
  void UpdateFlutterCursor(const std::string& cursor_name) override {
    if (view_properties_.use_mouse_cursor) {
      native_window_->UpdateCursor(cursor_name, pointer_x_, pointer_y_);
    }
  }

  // |FlutterWindowBindingHandler|
  void UpdateVirtualKeyboardStatus(const bool show) override {
    // currently not supported.
  }

  // |FlutterWindowBindingHandler|
  std::string GetClipboardData() override { return clipboard_data_; }

  // |FlutterWindowBindingHandler|
  void SetClipboardData(const std::string& data) override {
    clipboard_data_ = data;
  }

 protected:
  static constexpr libinput_interface kLibinputInterface = {
      .open_restricted =
          [](const char* path, int flags, void* user_data) -> int {
        auto ret = open(path, flags | O_CLOEXEC);
        if (ret == -1) {
          ELINUX_LOG(ERROR)
              << "Failed to open " << path << ", error: " << strerror(errno);
        }
        return ret;
      },
      .close_restricted = [](int fd, void* user_data) -> void { close(fd); },
  };

  bool RegisterUdevDrmEventLoop(const std::string& device_filename) {
    auto udev = udev_new();
    if (!udev) {
      ELINUX_LOG(ERROR) << "Failed to create udev instance.";
      return false;
    }

    constexpr char kUdevMonitorSystemUdev[] = "udev";
    udev_monitor_ = udev_monitor_new_from_netlink(udev, kUdevMonitorSystemUdev);
    if (!udev_monitor_) {
      ELINUX_LOG(ERROR) << "Failed to create udev monitor.";
      udev_unref(udev);
      return false;
    }

    constexpr char kUdevMonitorSubsystemDrm[] = "drm";
    if (udev_monitor_filter_add_match_subsystem_devtype(
            udev_monitor_, kUdevMonitorSubsystemDrm, NULL) < 0) {
      ELINUX_LOG(ERROR) << "Failed to filter udev monitor.";
      udev_unref(udev);
      return false;
    }

    constexpr char kFileNameSeparator[] = "/";
    auto pos = device_filename.find_last_of(kFileNameSeparator);
    if (pos == std::string::npos) {
      ELINUX_LOG(ERROR) << "Failed to get device name position.";
      udev_unref(udev);
      return false;
    }

    auto device_name = device_filename.substr(pos + 1);
    auto device = udev_device_new_from_subsystem_sysname(
        udev, kUdevMonitorSubsystemDrm, device_name.c_str());
    if (!device) {
      ELINUX_LOG(ERROR) << "Failed to get device from " << device_name;
      udev_unref(udev);
      return false;
    }

    auto sysnum = udev_device_get_sysnum(device);
    if (!sysnum) {
      ELINUX_LOG(ERROR) << "Failed to get device id.";
      udev_unref(udev);
      return false;
    }
    drm_device_id_ = std::atoi(sysnum);
    udev_unref(udev);

    if (sd_event_new(&udev_drm_event_loop_) < 0) {
      ELINUX_LOG(ERROR) << "Failed to create udev drm event loop.";
      return false;
    }

    if (sd_event_add_io(
            udev_drm_event_loop_, NULL, udev_monitor_get_fd(udev_monitor_),
            EPOLLIN | EPOLLRDHUP | EPOLLPRI, OnUdevDrmEvent, this) < 0) {
      ELINUX_LOG(ERROR) << "Failed to listen for udev drm event.";
      return false;
    }

    if (udev_monitor_enable_receiving(udev_monitor_) < 0) {
      ELINUX_LOG(ERROR) << "Failed to enable udev monitor receiving.";
      return false;
    }

    return true;
  }

  static int OnUdevDrmEvent(sd_event_source* source,
                            int fd,
                            uint32_t revents,
                            void* data) {
    auto self = reinterpret_cast<ELinuxWindowDrm*>(data);
    auto device = udev_monitor_receive_device(self->udev_monitor_);
    if (!device) {
      ELINUX_LOG(ERROR) << "Failed to receive udev device.";
      return -1;
    }

    if (self->IsUdevEventHotplug(*device) &&
        self->native_window_->ConfigureDisplay()) {
      auto width = self->native_window_->Width();
      auto height = self->native_window_->Height();
      if (self->current_rotation_ == 90 || self->current_rotation_ == 270) {
        std::swap(width, height);
      }
      if (self->view_properties_.width != width ||
          self->view_properties_.height != height) {
        self->view_properties_.width = width;
        self->view_properties_.height = height;
        ELINUX_LOG(INFO) << "Display output resolution: "
                         << self->view_properties_.width << "x"
                         << self->view_properties_.height;
        if (self->binding_handler_delegate_) {
          self->binding_handler_delegate_->OnWindowSizeChanged(
              self->view_properties_.width, self->view_properties_.height);
        }
      }
    }

    udev_device_unref(device);
    return 0;
  }

  bool IsUdevEventHotplug(udev_device& device) {
    auto sysnum = udev_device_get_sysnum(&device);
    if (!sysnum) {
      ELINUX_LOG(ERROR) << "Failed to get device id.";
      return false;
    } else if (std::atoi(sysnum) != drm_device_id_) {
      ELINUX_LOG(ERROR) << "Not expected device id.";
      return false;
    }

    constexpr char kUdevPropertyKeyHotplug[] = "HOTPLUG";
    auto value =
        udev_device_get_property_value(&device, kUdevPropertyKeyHotplug);
    if (!value) {
      ELINUX_LOG(ERROR) << "Failed to get udev device property value.";
      return false;
    }

    constexpr char kPropertyOn[] = "1";
    return std::strcmp(value, kPropertyOn) == 0;
  }

  static int OnLibinputEvent(sd_event_source* source,
                             int fd,
                             uint32_t revents,
                             void* data) {
    auto self = reinterpret_cast<ELinuxWindowDrm*>(data);
    auto ret = libinput_dispatch(self->libinput_);
    if (ret < 0) {
      ELINUX_LOG(ERROR) << "Failed to dispatch libinput events.";
      return -ret;
    }

    auto previous_pointer_x = self->pointer_x_;
    auto previous_pointer_y = self->pointer_y_;

    while (libinput_next_event_type(self->libinput_) != LIBINPUT_EVENT_NONE) {
      auto event = libinput_get_event(self->libinput_);
      auto event_type = libinput_event_get_type(event);

      switch (event_type) {
        case LIBINPUT_EVENT_DEVICE_ADDED:
          self->OnDeviceAdded(event);
          break;
        case LIBINPUT_EVENT_DEVICE_REMOVED:
          self->OnDeviceRemoved(event);
          break;
        case LIBINPUT_EVENT_KEYBOARD_KEY:
          self->OnKeyEvent(event);
          break;
        case LIBINPUT_EVENT_POINTER_MOTION:
          self->OnPointerMotion(event);
          break;
        case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
          self->OnPointerMotionAbsolute(event);
          break;
        case LIBINPUT_EVENT_POINTER_BUTTON:
          self->OnPointerButton(event);
          break;
        case LIBINPUT_EVENT_POINTER_AXIS:
          self->OnPointerAxis(event);
          break;
        case LIBINPUT_EVENT_TOUCH_DOWN:
          self->OnTouchDown(event);
          break;
        case LIBINPUT_EVENT_TOUCH_UP:
          self->OnTouchUp(event);
          break;
        case LIBINPUT_EVENT_TOUCH_MOTION:
          self->OnTouchMotion(event);
          break;
        case LIBINPUT_EVENT_TOUCH_CANCEL:
          self->OnTouchCancel(event);
          break;
        case LIBINPUT_EVENT_TOUCH_FRAME:
          // do nothing.
          break;
        default:
          break;
      }
      libinput_event_destroy(event);
    }

    if (self->view_properties_.use_mouse_cursor &&
        ((self->pointer_x_ != previous_pointer_x) ||
         (self->pointer_y_ != previous_pointer_y))) {
      self->native_window_->MoveCursor(self->pointer_x_, self->pointer_y_);
    }

    return 0;
  }

  void OnDeviceAdded(libinput_event* event) {
    auto device = libinput_event_get_device(event);
    auto device_data = std::make_unique<LibinputDeviceData>();
    size_t timestamp =
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch())
            .count();

    device_data->id = timestamp;
    device_data->is_pointer_device = false;
    libinput_device_set_user_data(device, device_data.get());
    libinput_devices_[timestamp] = std::move(device_data);
  }

  void OnDeviceRemoved(libinput_event* event) {
    auto device = libinput_event_get_device(event);
    auto device_data = reinterpret_cast<LibinputDeviceData*>(
        libinput_device_get_user_data(device));

    if (view_properties_.use_mouse_cursor &&
        libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_POINTER)) {
      if (device_data && device_data->is_pointer_device) {
        if (--libinput_pointer_devices_ == 0) {
          native_window_->DismissCursor();
        }
      }
    }

    if (device_data) {
      if (libinput_devices_.count(device_data->id) > 0) {
        libinput_devices_.erase(libinput_devices_.find(device_data->id));
      }
    }
  }

  void OnKeyEvent(libinput_event* event) {
    if (binding_handler_delegate_) {
      auto key_event = libinput_event_get_keyboard_event(event);
      auto evdev_keycode =
          static_cast<uint16_t>(libinput_event_keyboard_get_key(key_event));
      auto key_state = libinput_event_keyboard_get_key_state(key_event);
      binding_handler_delegate_->OnKey(evdev_keycode,
                                       key_state == LIBINPUT_KEY_STATE_PRESSED);
    }
  }

  void OnPointerMotion(libinput_event* event) {
    DetectPointerDevice(event);
    if (binding_handler_delegate_) {
      auto pointer_event = libinput_event_get_pointer_event(event);
      auto dx = libinput_event_pointer_get_dx(pointer_event);
      auto dy = libinput_event_pointer_get_dy(pointer_event);

      auto new_pointer_x = pointer_x_ + dx;
      new_pointer_x = std::max(0.0, new_pointer_x);
      new_pointer_x = std::min(static_cast<double>(view_properties_.width - 1),
                               new_pointer_x);
      auto new_pointer_y = pointer_y_ + dy;
      new_pointer_y = std::max(0.0, new_pointer_y);
      new_pointer_y = std::min(static_cast<double>(view_properties_.height - 1),
                               new_pointer_y);

      binding_handler_delegate_->OnPointerMove(new_pointer_x, new_pointer_y);
      pointer_x_ = new_pointer_x;
      pointer_y_ = new_pointer_y;
    }
  }

  void OnPointerMotionAbsolute(libinput_event* event) {
    DetectPointerDevice(event);
    if (binding_handler_delegate_) {
      auto pointer_event = libinput_event_get_pointer_event(event);
      auto x = libinput_event_pointer_get_absolute_x_transformed(
          pointer_event, view_properties_.width);
      auto y = libinput_event_pointer_get_absolute_y_transformed(
          pointer_event, view_properties_.height);

      binding_handler_delegate_->OnPointerMove(x, y);
      pointer_x_ = x;
      pointer_y_ = y;
    }
  }

  void OnPointerButton(libinput_event* event) {
    DetectPointerDevice(event);
    if (binding_handler_delegate_) {
      auto pointer_event = libinput_event_get_pointer_event(event);
      auto button = libinput_event_pointer_get_button(pointer_event);
      auto state = libinput_event_pointer_get_button_state(pointer_event);

      FlutterPointerMouseButtons flutter_button;
      switch (button) {
        case BTN_LEFT:
          flutter_button = kFlutterPointerButtonMousePrimary;
          break;
        case BTN_RIGHT:
          flutter_button = kFlutterPointerButtonMouseSecondary;
          break;
        case BTN_MIDDLE:
          flutter_button = kFlutterPointerButtonMouseMiddle;
          break;
        case BTN_BACK:
          flutter_button = kFlutterPointerButtonMouseBack;
          break;
        case BTN_FORWARD:
          flutter_button = kFlutterPointerButtonMouseForward;
          break;
        default:
          ELINUX_LOG(ERROR) << "Not expected button input: " << button;
          return;
      }

      if (state == LIBINPUT_BUTTON_STATE_PRESSED) {
        binding_handler_delegate_->OnPointerDown(pointer_x_, pointer_y_,
                                                 flutter_button);
      } else {
        binding_handler_delegate_->OnPointerUp(pointer_x_, pointer_y_,
                                               flutter_button);
      }
    }
  }

  void OnPointerAxis(libinput_event* event) {
    DetectPointerDevice(event);
    if (binding_handler_delegate_) {
      auto pointer_event = libinput_event_get_pointer_event(event);
      if (libinput_event_pointer_has_axis(
              pointer_event, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL)) {
        ProcessPointerAxis(pointer_event,
                           LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);
      }
      if (libinput_event_pointer_has_axis(
              pointer_event, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL)) {
        ProcessPointerAxis(pointer_event,
                           LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);
      }
    }
  }

  void DetectPointerDevice(libinput_event* event) {
    auto device = libinput_event_get_device(event);
    auto device_data = reinterpret_cast<LibinputDeviceData*>(
        libinput_device_get_user_data(device));

    // Shows the mouse cursor only when connecting mouse devices.
    // The mouse cursor is not displayed by touch inputs.
    if (device_data && !device_data->is_pointer_device) {
      device_data->is_pointer_device = true;
      libinput_pointer_devices_++;
      ShowMouseCursor();
    }
  }

  void ShowMouseCursor() {
    if (view_properties_.use_mouse_cursor && libinput_pointer_devices_ == 1) {
      // When launching the application, either route will be used depending on
      // the timing.
      if (native_window_) {
        native_window_->ShowCursor(pointer_x_, pointer_y_);
      } else {
        is_pending_cursor_add_event_ = true;
      }
    }
  }

  void OnTouchDown(libinput_event* event) {
    if (binding_handler_delegate_) {
      auto touch_event = libinput_event_get_touch_event(event);
      auto time = libinput_event_touch_get_time(touch_event);
      auto slot = libinput_event_touch_get_seat_slot(touch_event);
      auto x = libinput_event_touch_get_x_transformed(touch_event,
                                                      view_properties_.width);
      auto y = libinput_event_touch_get_y_transformed(touch_event,
                                                      view_properties_.height);
      binding_handler_delegate_->OnTouchDown(time, slot, x, y);
    }
  }

  void OnTouchUp(libinput_event* event) {
    if (binding_handler_delegate_) {
      auto touch_event = libinput_event_get_touch_event(event);
      auto time = libinput_event_touch_get_time(touch_event);
      auto slot = libinput_event_touch_get_seat_slot(touch_event);
      binding_handler_delegate_->OnTouchUp(time, slot);
    }
  }

  void OnTouchMotion(libinput_event* event) {
    if (binding_handler_delegate_) {
      auto touch_event = libinput_event_get_touch_event(event);
      auto time = libinput_event_touch_get_time(touch_event);
      auto slot = libinput_event_touch_get_seat_slot(touch_event);
      auto x = libinput_event_touch_get_x_transformed(touch_event,
                                                      view_properties_.width);
      auto y = libinput_event_touch_get_y_transformed(touch_event,
                                                      view_properties_.height);
      binding_handler_delegate_->OnTouchMotion(time, slot, x, y);
    }
  }

  void OnTouchCancel(libinput_event* event) {
    if (binding_handler_delegate_) {
      binding_handler_delegate_->OnTouchCancel();
    }
  }

  void ProcessPointerAxis(libinput_event_pointer* pointer_event,
                          libinput_pointer_axis axis) {
    auto source = libinput_event_pointer_get_axis_source(pointer_event);
    double value = 0.0;
    switch (source) {
      case LIBINPUT_POINTER_AXIS_SOURCE_WHEEL:
        /* libinput < 0.8 sent wheel click events with value 10. Since 0.8
           the value is the angle of the click in degrees. To keep
           backwards-compat with existing clients, we just send multiples of
           the click count.
         */
        value = 10 * libinput_event_pointer_get_axis_value_discrete(
                         pointer_event, axis);
        break;
      case LIBINPUT_POINTER_AXIS_SOURCE_FINGER:
      case LIBINPUT_POINTER_AXIS_SOURCE_CONTINUOUS:
        value = libinput_event_pointer_get_axis_value(pointer_event, axis);
        break;
      default:
        ELINUX_LOG(ERROR) << "Not expected axis source: " << source;
        return;
    }

    constexpr int32_t kScrollOffsetMultiplier = 20;
    binding_handler_delegate_->OnScroll(
        pointer_x_, pointer_y_,
        axis == LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL ? 0 : value,
        axis == LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL ? value : 0,
        kScrollOffsetMultiplier);
  }

  struct LibinputDeviceData {
    size_t id;
    bool is_pointer_device;
  };

  // A pointer to a FlutterWindowsView that can be used to update engine
  // windowing and input state.
  WindowBindingHandlerDelegate* binding_handler_delegate_ = nullptr;

  std::unique_ptr<T> native_window_;
  std::unique_ptr<SurfaceGl> render_surface_;

  bool display_valid_;
  bool is_pending_cursor_add_event_;
  sd_event* libinput_event_loop_;
  libinput* libinput_;
  std::unordered_map<size_t, std::unique_ptr<LibinputDeviceData>>
      libinput_devices_;
  int libinput_pointer_devices_ = 0;

  sd_event* udev_drm_event_loop_ = nullptr;
  udev_monitor* udev_monitor_ = nullptr;
  int drm_device_id_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_ELINUX_WINDOW_DRM_H_
