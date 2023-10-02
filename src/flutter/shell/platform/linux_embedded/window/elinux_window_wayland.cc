// Copyright 2023 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/window/elinux_window_wayland.h"

#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <poll.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <unordered_map>

#include "flutter/shell/platform/linux_embedded/logger.h"
#include "flutter/shell/platform/linux_embedded/surface/context_egl.h"

namespace flutter {

namespace {
constexpr char kZwpTextInputManagerV1[] = "zwp_text_input_manager_v1";
constexpr char kZwpTextInputManagerV3[] = "zwp_text_input_manager_v3";
constexpr char kZxdgDecorationManagerV1[] = "zxdg_decoration_manager_v1";

constexpr char kWlCursorThemeBottomLeftCorner[] = "bottom_left_corner";
constexpr char kWlCursorThemeBottomRightCorner[] = "bottom_right_corner";
constexpr char kWlCursorThemeBottomSide[] = "bottom_side";
constexpr char kWlCursorThemeGrabbing[] = "grabbing";
constexpr char kWlCursorThemeLeftPtr[] = "left_ptr";
constexpr char kWlCursorThemeLeftSide[] = "left_side";
constexpr char kWlCursorThemeRightSide[] = "right_side";
constexpr char kWlCursorThemeTopLeftCorner[] = "top_left_corner";
constexpr char kWlCursorThemeTopRightCorner[] = "top_right_corner";
constexpr char kWlCursorThemeTopSide[] = "top_side";
constexpr char kWlCursorThemeXterm[] = "xterm";
constexpr char kWlCursorThemeHand1[] = "hand1";
constexpr char kWlCursorThemeWatch[] = "watch";
constexpr char kCursorNameNone[] = "none";

constexpr char kClipboardMimeTypeText[] = "text/plain";
}  // namespace

const wl_registry_listener ELinuxWindowWayland::kWlRegistryListener = {
    .global =
        [](void* data,
           wl_registry* wl_registry,
           uint32_t name,
           const char* interface,
           uint32_t version) {
          ELINUX_LOG(TRACE) << "wl_registry_listener.global";

          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
          self->WlRegistryHandler(wl_registry, name, interface, version);
        },
    .global_remove =
        [](void* data, wl_registry* wl_registry, uint32_t name) {
          ELINUX_LOG(TRACE) << "wl_registry_listener.global_remove";

          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
          self->WlUnRegistryHandler(wl_registry, name);
        },
};

const xdg_wm_base_listener ELinuxWindowWayland::kXdgWmBaseListener = {
    .ping =
        [](void* data, xdg_wm_base* xdg_wm_base, uint32_t serial) {
          ELINUX_LOG(TRACE) << "xdg_wm_base_listener.ping";
          xdg_wm_base_pong(xdg_wm_base, serial);
        },
};

const xdg_surface_listener ELinuxWindowWayland::kXdgSurfaceListener = {
    .configure =
        [](void* data, xdg_surface* xdg_surface, uint32_t serial) {
          ELINUX_LOG(TRACE) << "xdg_surface_listener.configure";

          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
          if (self->window_decorations_) {
            // Shift the window position to the bottom to show decoration
            // even when the window is displayed in the upper left corner
            // of the screen
            constexpr int32_t x = 0;
            int32_t y = 0;
            y = -self->window_decorations_->Height();

            auto width = self->view_properties_.width;
            auto height = self->view_properties_.height;
            if (!self->maximised_) {
              height -= y;
            }

            // clip
            if (self->display_max_height_ > 0) {
              height = std::min(height, self->display_max_height_);
            }

            xdg_surface_set_window_geometry(xdg_surface, x, y, width, height);
          }

          xdg_surface_ack_configure(xdg_surface, serial);
          if (self->wait_for_configure_) {
            self->wait_for_configure_ = false;
          }
          self->request_redraw_ = true;
        },
};

const xdg_toplevel_listener ELinuxWindowWayland::kXdgToplevelListener = {
    .configure =
        [](void* data,
           xdg_toplevel* xdg_toplevel,
           int32_t width,
           int32_t height,
           wl_array* states) {
          ELINUX_LOG(TRACE)
              << "xdg_toplevel_listener.configure: " << width << ", " << height;

          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
          if (self->current_rotation_ == 90 || self->current_rotation_ == 270) {
            std::swap(width, height);
          }

          int32_t next_width_dip = width / self->current_scale_;
          int32_t next_height_dip = height / self->current_scale_;
          if (self->restore_window_required_) {
            self->restore_window_required_ = false;
            next_width_dip = self->restore_window_width_;
            next_height_dip = self->restore_window_height_;
          }

          if (!next_width_dip || !next_height_dip ||
              (self->view_properties_.width == next_width_dip &&
               self->view_properties_.height == next_height_dip)) {
            return;
          }

          ELINUX_LOG(TRACE) << "request redraw: " << next_width_dip << ", "
                            << next_height_dip;
          self->view_properties_.width = next_width_dip;
          self->view_properties_.height = next_height_dip;
          self->request_redraw_ = true;
        },
    .close =
        [](void* data, xdg_toplevel* xdg_toplevel) {
          ELINUX_LOG(TRACE) << "xdg_toplevel_listener.close";

          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
          self->running_ = false;
        }};

const wl_surface_listener ELinuxWindowWayland::kWlSurfaceListener = {
    .enter =
        [](void* data, wl_surface* wl_surface, wl_output* output) {
          ELINUX_LOG(TRACE) << "wl_surface_listener.enter";

          // The compositor can send a null output: crbug.com/1332540
          if (!output) {
            ELINUX_LOG(ERROR) << "cannot enter a NULL output";
            return;
          }

          const uint32_t output_id =
              wl_proxy_get_id(reinterpret_cast<wl_proxy*>(output));
          ELINUX_LOG(TRACE) << "window entered output " << output_id;
          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);

          self->entered_outputs_.insert(output_id);
          self->UpdateWindowScale();
        },
    .leave =
        [](void* data, wl_surface* wl_surface, wl_output* output) {
          ELINUX_LOG(TRACE) << "wl_surface_listener.leave";

          // The compositor can send a null output: crbug.com/1332540
          if (!output) {
            ELINUX_LOG(ERROR) << "cannot leave a NULL output";
            return;
          }

          const uint32_t output_id =
              wl_proxy_get_id(reinterpret_cast<wl_proxy*>(output));
          ELINUX_LOG(TRACE) << "window left output " << output_id;
          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);

          self->entered_outputs_.erase(output_id);
          self->UpdateWindowScale();
        }};

const wp_presentation_listener ELinuxWindowWayland::kWpPresentationListener = {
    .clock_id =
        [](void* data, wp_presentation* wp_presentation, uint32_t clk_id) {
          ELINUX_LOG(TRACE) << "wp_presentation_listener.clock_id";

          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
          self->wp_presentation_clk_id_ = clk_id;
          ELINUX_LOG(TRACE) << "clk_id = " << clk_id;
        },
};

const wp_presentation_feedback_listener
    ELinuxWindowWayland::kWpPresentationFeedbackListener = {
        .sync_output =
            [](void* data,
               struct wp_presentation_feedback* wp_presentation_feedback,
               wl_output* output) {
              ELINUX_LOG(TRACE)
                  << "wp_presentation_feedback_listener.sync_output";
            },
        .presented =
            [](void* data,
               struct wp_presentation_feedback* wp_presentation_feedback,
               uint32_t tv_sec_hi,
               uint32_t tv_sec_lo,
               uint32_t tv_nsec,
               uint32_t refresh,
               uint32_t seq_hi,
               uint32_t seq_lo,
               uint32_t flags) {
              ELINUX_LOG(TRACE)
                  << "wp_presentation_feedback_listener.presented";

              auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
              self->last_frame_time_nanos_ =
                  (((static_cast<uint64_t>(tv_sec_hi) << 32) + tv_sec_lo) *
                   1000000000) +
                  tv_nsec;
              self->frame_rate_ =
                  static_cast<int32_t>(std::round(1000000000000.0 / refresh));

              if (self->window_decorations_) {
                self->window_decorations_->Draw();
              }

              wp_presentation_feedback_add_listener(
                  ::wp_presentation_feedback(self->wp_presentation_,
                                             self->native_window_->Surface()),
                  &kWpPresentationFeedbackListener, data);
            },
        .discarded =
            [](void* data,
               struct wp_presentation_feedback* wp_presentation_feedback) {
              ELINUX_LOG(TRACE)
                  << "wp_presentation_feedback_listener.discarded";

              auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
              if (self->window_decorations_) {
                self->window_decorations_->Draw();
              }

              wp_presentation_feedback_add_listener(
                  ::wp_presentation_feedback(self->wp_presentation_,
                                             self->native_window_->Surface()),
                  &kWpPresentationFeedbackListener, data);
            },
};

const wl_callback_listener ELinuxWindowWayland::kWlSurfaceFrameListener = {
    .done =
        [](void* data, wl_callback* wl_callback, uint32_t time) {
          ELINUX_LOG(TRACE) << "wl_callback_listener.done";

          // The presentation-time is an extended protocol and isn't supported
          // by all compositors. This path is for when it wasn't supported.
          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
          if (self->wp_presentation_clk_id_ != UINT32_MAX) {
            return;
          }

          if (self->window_decorations_) {
            self->window_decorations_->Draw();
          }

          self->last_frame_time_nanos_ = static_cast<uint64_t>(time) * 1000000;

          auto callback = wl_surface_frame(self->native_window_->Surface());
          wl_callback_destroy(wl_callback);
          wl_callback_add_listener(callback, &kWlSurfaceFrameListener, data);
        },
};

const wl_seat_listener ELinuxWindowWayland::kWlSeatListener = {
    .capabilities = [](void* data, wl_seat* seat, uint32_t caps) -> void {
      ELINUX_LOG(TRACE) << "wl_seat_listener.capabilities";

      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      if ((caps & WL_SEAT_CAPABILITY_POINTER) && !self->wl_pointer_) {
        self->wl_pointer_ = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(self->wl_pointer_, &kWlPointerListener, self);
      } else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && self->wl_pointer_) {
        wl_pointer_destroy(self->wl_pointer_);
        self->wl_pointer_ = nullptr;
      }

      if ((caps & WL_SEAT_CAPABILITY_TOUCH) && !self->wl_touch_) {
        self->wl_touch_ = wl_seat_get_touch(seat);
        wl_touch_add_listener(self->wl_touch_, &kWlTouchListener, self);
      } else if (!(caps & WL_SEAT_CAPABILITY_TOUCH) && self->wl_touch_) {
        wl_touch_destroy(self->wl_touch_);
        self->wl_touch_ = nullptr;
      }

      if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !self->wl_keyboard_) {
        self->wl_keyboard_ = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(self->wl_keyboard_, &kWlKeyboardListener,
                                 self);
      } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && self->wl_keyboard_) {
        wl_keyboard_destroy(self->wl_keyboard_);
        self->wl_keyboard_ = nullptr;
      }
    },
    .name = [](void* data, struct wl_seat* wl_seat, const char* name) -> void {
      ELINUX_LOG(TRACE) << "wl_seat_listener.name";
    },
};

const wl_pointer_listener ELinuxWindowWayland::kWlPointerListener = {
    .enter = [](void* data,
                wl_pointer* wl_pointer,
                uint32_t serial,
                wl_surface* surface,
                wl_fixed_t surface_x,
                wl_fixed_t surface_y) -> void {
      ELINUX_LOG(TRACE) << "wl_pointer_listener.enter";

      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      self->wl_current_surface_ = surface;
      self->serial_ = serial;

      if (self->view_properties_.use_mouse_cursor) {
        self->cursor_info_.pointer = self->wl_pointer_;
        self->cursor_info_.serial = serial;
      }

      if (self->binding_handler_delegate_) {
        double x_px = wl_fixed_to_double(surface_x) * self->current_scale_;
        double y_px = wl_fixed_to_double(surface_y) * self->current_scale_;
        self->binding_handler_delegate_->OnPointerMove(x_px, y_px);
        self->pointer_x_ = x_px;
        self->pointer_y_ = y_px;
      }
    },
    .leave = [](void* data,
                wl_pointer* pointer,
                uint32_t serial,
                wl_surface* surface) -> void {
      ELINUX_LOG(TRACE) << "wl_pointer_listener.leave";

      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      self->wl_current_surface_ = surface;
      self->serial_ = serial;

      if (self->view_properties_.use_mouse_cursor) {
        // Clear the cursor name in order to make sure it gets redrawn next time
        // it enters the surface.
        self->cursor_info_.cursor_name.clear();
      }

      if (self->binding_handler_delegate_) {
        self->binding_handler_delegate_->OnPointerLeave();
        self->pointer_x_ = -1;
        self->pointer_y_ = -1;
      }
    },
    .motion = [](void* data,
                 wl_pointer* pointer,
                 uint32_t time,
                 wl_fixed_t surface_x,
                 wl_fixed_t surface_y) -> void {
      ELINUX_LOG(TRACE) << "wl_pointer_listener.motion";

      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      if (self->binding_handler_delegate_) {
        double x_px = wl_fixed_to_double(surface_x) * self->current_scale_;
        double y_px = wl_fixed_to_double(surface_y) * self->current_scale_;
        self->binding_handler_delegate_->OnPointerMove(x_px, y_px);
        self->pointer_x_ = x_px;
        self->pointer_y_ = y_px;
      }
    },
    .button = [](void* data,
                 wl_pointer* pointer,
                 uint32_t serial,
                 uint32_t time,
                 uint32_t button,
                 uint32_t status) -> void {
      ELINUX_LOG(TRACE) << "wl_pointer_listener.button";

      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      self->serial_ = serial;

      if (button == BTN_LEFT && status == WL_POINTER_BUTTON_STATE_PRESSED) {
        if (self->window_decorations_ &&
            self->window_decorations_->IsMatched(
                self->wl_current_surface_,
                WindowDecoration::DecorationType::TITLE_BAR)) {
          xdg_toplevel_move(self->xdg_toplevel_, self->wl_seat_, serial);
          return;
        }

        if (self->window_decorations_ &&
            self->window_decorations_->IsMatched(
                self->wl_current_surface_,
                WindowDecoration::DecorationType::CLOSE_BUTTON)) {
          self->running_ = false;
          return;
        }

        if (self->window_decorations_ &&
            self->window_decorations_->IsMatched(
                self->wl_current_surface_,
                WindowDecoration::DecorationType::MAXIMISE_BUTTON)) {
          if (self->maximised_) {
            xdg_toplevel_unset_maximized(self->xdg_toplevel_);

            // Requests to return to the original window size before maximizing.
            self->restore_window_required_ = true;
          } else {
            // Stores original window size.
            self->restore_window_width_ = self->view_properties_.width;
            self->restore_window_height_ = self->view_properties_.height;
            self->restore_window_required_ = false;

            xdg_toplevel_set_maximized(self->xdg_toplevel_);
          }
          self->maximised_ = !self->maximised_;
          return;
        }

        if (self->window_decorations_ &&
            self->window_decorations_->IsMatched(
                self->wl_current_surface_,
                flutter::WindowDecoration::DecorationType::MINIMISE_BUTTON)) {
          xdg_toplevel_set_minimized(self->xdg_toplevel_);
          return;
        }
      }

      if (self->binding_handler_delegate_) {
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

        if (status == WL_POINTER_BUTTON_STATE_PRESSED) {
          self->binding_handler_delegate_->OnPointerDown(
              self->pointer_x_, self->pointer_y_, flutter_button);
        } else {
          self->binding_handler_delegate_->OnPointerUp(
              self->pointer_x_, self->pointer_y_, flutter_button);
        }
      }
    },
    .axis = [](void* data,
               wl_pointer* wl_pointer,
               uint32_t time,
               uint32_t axis,
               wl_fixed_t value) -> void {
      ELINUX_LOG(TRACE) << "wl_pointer_listener.axis";

      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      if (self->binding_handler_delegate_) {
        double delta = wl_fixed_to_double(value);
        constexpr int32_t kScrollOffsetMultiplier = 20;
        self->binding_handler_delegate_->OnScroll(
            self->pointer_x_, self->pointer_y_,
            axis == WL_POINTER_AXIS_VERTICAL_SCROLL ? 0 : delta,
            axis == WL_POINTER_AXIS_VERTICAL_SCROLL ? delta : 0,
            kScrollOffsetMultiplier);
      }
    },
};

const wl_touch_listener ELinuxWindowWayland::kWlTouchListener = {
    .down = [](void* data,
               wl_touch* wl_touch,
               uint32_t serial,
               uint32_t time,
               wl_surface* surface,
               int32_t id,
               wl_fixed_t surface_x,
               wl_fixed_t surface_y) -> void {
      ELINUX_LOG(TRACE) << "wl_touch_listener.down";

      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      self->serial_ = serial;
      if (self->binding_handler_delegate_) {
        double x = wl_fixed_to_double(surface_x);
        double y = wl_fixed_to_double(surface_y);
        self->binding_handler_delegate_->OnTouchDown(time, id, x, y);
      }
    },
    .up = [](void* data,
             wl_touch* wl_touch,
             uint32_t serial,
             uint32_t time,
             int32_t id) -> void {
      ELINUX_LOG(TRACE) << "wl_touch_listener.up";

      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      self->serial_ = serial;
      if (self->binding_handler_delegate_) {
        self->binding_handler_delegate_->OnTouchUp(time, id);
      }
    },
    .motion = [](void* data,
                 wl_touch* wl_touch,
                 uint32_t time,
                 int32_t id,
                 wl_fixed_t surface_x,
                 wl_fixed_t surface_y) -> void {
      ELINUX_LOG(TRACE) << "wl_touch_listener.motion";

      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      if (self->binding_handler_delegate_) {
        double x = wl_fixed_to_double(surface_x);
        double y = wl_fixed_to_double(surface_y);
        self->binding_handler_delegate_->OnTouchMotion(time, id, x, y);
      }
    },
    .frame = [](void* data, wl_touch* wl_touch) -> void {},
    .cancel = [](void* data, wl_touch* wl_touch) -> void {
      ELINUX_LOG(TRACE) << "wl_touch_listener.cancel";

      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      if (self->binding_handler_delegate_) {
        self->binding_handler_delegate_->OnTouchCancel();
      }
    },
};

const wl_keyboard_listener ELinuxWindowWayland::kWlKeyboardListener = {
    .keymap = [](void* data,
                 wl_keyboard* wl_keyboard,
                 uint32_t format,
                 int fd,
                 uint32_t size) -> void {
      ELINUX_LOG(TRACE) << "wl_keyboard_listener.keymap";

      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      assert(format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1);
      if (self->binding_handler_delegate_) {
        self->binding_handler_delegate_->OnKeyMap(format, fd, size);
      }
    },
    .enter = [](void* data,
                wl_keyboard* wl_keyboard,
                uint32_t serial,
                wl_surface* surface,
                wl_array* keys) -> void {
      ELINUX_LOG(TRACE) << "wl_keyboard_listener.enter";

      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      self->serial_ = serial;
    },
    .leave = [](void* data,
                wl_keyboard* wl_keyboard,
                uint32_t serial,
                wl_surface* surface) -> void {
      ELINUX_LOG(TRACE) << "wl_keyboard_listener.leave";

      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      self->serial_ = serial;
    },
    .key = [](void* data,
              wl_keyboard* wl_keyboard,
              uint32_t serial,
              uint32_t time,
              uint32_t key,
              uint32_t state) -> void {
      ELINUX_LOG(TRACE) << "wl_keyboard_listener.key";

      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      self->serial_ = serial;
      if (self->binding_handler_delegate_) {
        self->binding_handler_delegate_->OnKey(
            key, state == WL_KEYBOARD_KEY_STATE_PRESSED);
      }
    },
    .modifiers = [](void* data,
                    wl_keyboard* wl_keyboard,
                    uint32_t serial,
                    uint32_t mods_depressed,
                    uint32_t mods_latched,
                    uint32_t mods_locked,
                    uint32_t group) -> void {
      ELINUX_LOG(TRACE) << "wl_keyboard_listener.modifiers";

      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      if (self->binding_handler_delegate_) {
        self->binding_handler_delegate_->OnKeyModifiers(
            mods_depressed, mods_latched, mods_locked, group);
      }
    },
    .repeat_info = [](void* data, wl_keyboard* wl_keyboard, int rate, int delay)
        -> void { ELINUX_LOG(TRACE) << "wl_keyboard_listener.repeat_info"; },
};

const wl_output_listener ELinuxWindowWayland::kWlOutputListener = {
    .geometry = [](void* data,
                   wl_output* wl_output,
                   int32_t x,
                   int32_t y,
                   int32_t physical_width,
                   int32_t physical_height,
                   int32_t subpixel,
                   const char* make,
                   const char* model,
                   int32_t output_transform) -> void {
      ELINUX_LOG(TRACE) << "wl_output_listener.geometry";
    },
    .mode = [](void* data,
               wl_output* wl_output,
               uint32_t flags,
               int32_t width,
               int32_t height,
               int32_t refresh) -> void {
      ELINUX_LOG(TRACE) << "wl_output_listener.mode";

      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      if (flags & WL_OUTPUT_MODE_CURRENT) {
        if (self->current_rotation_ == 90 || self->current_rotation_ == 270) {
          std::swap(width, height);
        }

        ELINUX_LOG(INFO) << "Display output info: width = " << width
                         << ", height = " << height
                         << ", refresh = " << refresh;
        self->display_max_width_ = width;
        self->display_max_height_ = height;

        // Some composers send 0 for the refresh value.
        if (refresh != 0) {
          self->frame_rate_ = refresh;
        }

        if (self->view_properties_.view_mode ==
            FlutterDesktopViewMode::kFullscreen) {
          self->view_properties_.width = width;
          self->view_properties_.height = height;
          self->request_redraw_ = true;
        }

        if (self->view_properties_.width > width) {
          ELINUX_LOG(WARNING)
              << "Requested width size(" << self->view_properties_.width << ") "
              << "is larger than display size(" << width
              << ")"
                 ", clipping";
          self->view_properties_.width = width;
          self->request_redraw_ = true;
        }
        if (self->view_properties_.height > height) {
          ELINUX_LOG(WARNING) << "Requested height size("
                              << self->view_properties_.height << ") "
                              << "is larger than display size(" << height
                              << ")"
                                 ", clipping";
          self->view_properties_.height = height;
          self->request_redraw_ = true;
        }
      }
    },
    .done = [](void* data, wl_output* wl_output) -> void {
      ELINUX_LOG(TRACE) << "wl_output_listener.done";
    },
    .scale = [](void* data, wl_output* wl_output, int32_t scale) -> void {
      ELINUX_LOG(TRACE) << "wl_output_listener.scale";

      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      const uint32_t output_id =
          wl_proxy_get_id(reinterpret_cast<wl_proxy*>(wl_output));

      ELINUX_LOG(INFO) << "Display scale for output(" << output_id
                       << "): " << scale;

      self->wl_output_scale_factors_[output_id] = scale;

      self->UpdateWindowScale();
    },
};

const zwp_text_input_v1_listener ELinuxWindowWayland::kZwpTextInputV1Listener =
    {
        .enter = [](void* data,
                    zwp_text_input_v1* zwp_text_input_v1,
                    wl_surface* surface) -> void {
          ELINUX_LOG(TRACE) << "zwp_text_input_v1_listener.enter";

          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
          // If there is no input data, the backspace key cannot be used,
          // so set dummy data.
          if (self->zwp_text_input_v1_) {
            zwp_text_input_v1_set_surrounding_text(self->zwp_text_input_v1_,
                                                   " ", 1, 1);
          }
        },
        .leave = [](void* data, zwp_text_input_v1* zwp_text_input_v1) -> void {
          ELINUX_LOG(TRACE) << "zwp_text_input_v1_listener.leave";

          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
          if (self->zwp_text_input_v1_) {
            zwp_text_input_v1_hide_input_panel(self->zwp_text_input_v1_);
          }
        },
        .modifiers_map = [](void* data,
                            zwp_text_input_v1* zwp_text_input_v1,
                            wl_array* map) -> void {
          ELINUX_LOG(TRACE) << "zwp_text_input_v1_listener.modifiers_map";
        },
        .input_panel_state = [](void* data,
                                zwp_text_input_v1* zwp_text_input_v1,
                                uint32_t state) -> void {
          ELINUX_LOG(TRACE) << "zwp_text_input_v1_listener.input_panel_state";
        },
        .preedit_string = [](void* data,
                             zwp_text_input_v1* zwp_text_input_v1,
                             uint32_t serial,
                             const char* text,
                             const char* commit) -> void {
          ELINUX_LOG(TRACE) << "zwp_text_input_v1_listener.preedit_string";

          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
          if (self->binding_handler_delegate_ && strlen(text)) {
            self->binding_handler_delegate_->OnVirtualKey(text[0]);
          }
          if (self->zwp_text_input_v1_) {
            zwp_text_input_v1_reset(self->zwp_text_input_v1_);
            // If there is no input data, the backspace key cannot be used,
            // so set dummy data.
            zwp_text_input_v1_set_surrounding_text(self->zwp_text_input_v1_,
                                                   " ", 1, 1);
          }
        },
        .preedit_styling = [](void* data,
                              zwp_text_input_v1* zwp_text_input_v1,
                              uint32_t index,
                              uint32_t length,
                              uint32_t style) -> void {
          ELINUX_LOG(TRACE) << "zwp_text_input_v1_listener.preedit_styling";
        },
        .preedit_cursor = [](void* data,
                             zwp_text_input_v1* zwp_text_input_v1,
                             int32_t index) -> void {
          ELINUX_LOG(TRACE) << "zwp_text_input_v1_listener.preedit_cursor";
        },
        .commit_string = [](void* data,
                            zwp_text_input_v1* zwp_text_input_v1,
                            uint32_t serial,
                            const char* text) -> void {
          ELINUX_LOG(TRACE) << "zwp_text_input_v1_listener.commit_string";

          // commit_string is notified only when the space key is pressed.
          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
          if (self->binding_handler_delegate_ && strlen(text)) {
            self->binding_handler_delegate_->OnVirtualKey(text[0]);
          }
          // If there is no input data, the backspace key cannot be used,
          // so set dummy data.
          if (self->zwp_text_input_v1_) {
            zwp_text_input_v1_set_surrounding_text(self->zwp_text_input_v1_,
                                                   " ", 1, 1);
          }
        },
        .cursor_position = [](void* data,
                              zwp_text_input_v1* zwp_text_input_v1,
                              int32_t index,
                              int32_t anchor) -> void {
          ELINUX_LOG(TRACE) << "zwp_text_input_v1_listener.cursor_position";
        },
        .delete_surrounding_text = [](void* data,
                                      zwp_text_input_v1* zwp_text_input_v1,
                                      int32_t index,
                                      uint32_t length) -> void {
          ELINUX_LOG(TRACE)
              << "zwp_text_input_v1_listener.delete_surrounding_text";

          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
          if (self->binding_handler_delegate_) {
            self->binding_handler_delegate_->OnVirtualSpecialKey(KEY_BACKSPACE);
          }
          // If there is no input data, the backspace key cannot be used,
          // so set dummy data.
          if (self->zwp_text_input_v1_) {
            zwp_text_input_v1_set_surrounding_text(self->zwp_text_input_v1_,
                                                   " ", 1, 1);
          }
        },
        .keysym = [](void* data,
                     zwp_text_input_v1* zwp_text_input_v1,
                     uint32_t serial,
                     uint32_t time,
                     uint32_t sym,
                     uint32_t state,
                     uint32_t modifiers) -> void {
          ELINUX_LOG(TRACE) << "zwp_text_input_v1_listener.keysym";

          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
          if ((state == WL_KEYBOARD_KEY_STATE_PRESSED) &&
              (self->binding_handler_delegate_)) {
            switch (sym) {
              case XKB_KEY_Left:
                self->binding_handler_delegate_->OnVirtualSpecialKey(KEY_LEFT);
                break;
              case XKB_KEY_Right:
                self->binding_handler_delegate_->OnVirtualSpecialKey(KEY_RIGHT);
                break;
              case XKB_KEY_Up:
                self->binding_handler_delegate_->OnVirtualSpecialKey(KEY_UP);
                break;
              case XKB_KEY_Down:
                self->binding_handler_delegate_->OnVirtualSpecialKey(KEY_DOWN);
                break;
              case XKB_KEY_Tab:
                self->binding_handler_delegate_->OnVirtualSpecialKey(KEY_TAB);
                break;
              case XKB_KEY_Return:
                self->binding_handler_delegate_->OnVirtualSpecialKey(KEY_ENTER);
                break;
              default:
                break;
            }
          }
        },
        .language = [](void* data,
                       zwp_text_input_v1* zwp_text_input_v1,
                       uint32_t serial,
                       const char* language) -> void {
          ELINUX_LOG(TRACE) << "zwp_text_input_v1_listener.language";
        },
        .text_direction = [](void* data,
                             zwp_text_input_v1* zwp_text_input_v1,
                             uint32_t serial,
                             uint32_t direction) -> void {
          ELINUX_LOG(TRACE) << "zwp_text_input_v1_listener.text_direction";
        },
};

const zwp_text_input_v3_listener ELinuxWindowWayland::kZwpTextInputV3Listener =
    {
        .enter = [](void* data,
                    zwp_text_input_v3* zwp_text_input_v3,
                    wl_surface* surface) -> void {
          ELINUX_LOG(TRACE) << "zwp_text_input_v3_listener.enter";

          // To appear the on-screen keyboard when the user returns to a Flutter
          // app which needs to show the on-screen keyboard.
          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
          if (self->is_requested_show_virtual_keyboard_) {
            self->ShowVirtualKeyboard();
          }
        },
        .leave = [](void* data,
                    zwp_text_input_v3* zwp_text_input_v3,
                    wl_surface* surface) -> void {
          ELINUX_LOG(TRACE) << "zwp_text_input_v3_listener.leave";
        },
        .preedit_string = [](void* data,
                             zwp_text_input_v3* zwp_text_input_v3,
                             const char* text,
                             int32_t cursor_begin,
                             int32_t cursor_end) -> void {
          ELINUX_LOG(TRACE) << "zwp_text_input_v3_listener.preedit_string";
        },
        .commit_string = [](void* data,
                            zwp_text_input_v3* zwp_text_input_v3,
                            const char* text) -> void {
          ELINUX_LOG(TRACE) << "zwp_text_input_v3_listener.commit_string";

          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
          if (self->binding_handler_delegate_ && strlen(text)) {
            self->binding_handler_delegate_->OnVirtualKey(text[0]);
          }
        },
        .delete_surrounding_text = [](void* data,
                                      zwp_text_input_v3* zwp_text_input_v3,
                                      uint32_t before_length,
                                      uint32_t after_length) -> void {
          ELINUX_LOG(TRACE)
              << "zwp_text_input_v3_listener.delete_surrounding_text";
        },
        .done = [](void* data,
                   zwp_text_input_v3* zwp_text_input_v3,
                   uint32_t serial) -> void {
          ELINUX_LOG(TRACE) << "zwp_text_input_v3_listener.done";
        },
};

const wl_data_device_listener ELinuxWindowWayland::kWlDataDeviceListener = {
    .data_offer = [](void* data,
                     wl_data_device* wl_data_device,
                     wl_data_offer* offer) -> void {
      ELINUX_LOG(TRACE) << "wl_data_device_listener.data_offer";
    },
    .enter = [](void* data,
                wl_data_device* wl_data_device,
                uint32_t serial,
                wl_surface* surface,
                wl_fixed_t x,
                wl_fixed_t y,
                wl_data_offer* offer) -> void {
      ELINUX_LOG(TRACE) << "wl_data_device_listener.enter";
    },
    .leave = [](void* data, wl_data_device* wl_data_device) -> void {
      ELINUX_LOG(TRACE) << "wl_data_device_listener.leave";
    },
    .motion = [](void* data,
                 wl_data_device* wl_data_device,
                 uint32_t time,
                 wl_fixed_t x,
                 wl_fixed_t y) -> void {
      ELINUX_LOG(TRACE) << "wl_data_device_listener.motion";
    },
    .drop = [](void* data, wl_data_device* wl_data_device) -> void {
      ELINUX_LOG(TRACE) << "wl_data_device_listener.drop";
    },
    .selection = [](void* data,
                    wl_data_device* wl_data_device,
                    wl_data_offer* offer) -> void {
      ELINUX_LOG(TRACE) << "wl_data_device_listener.selection";

      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      if (self->wl_data_offer_) {
        wl_data_offer_destroy(self->wl_data_offer_);
      }
      self->wl_data_offer_ = offer;
    },
};

const wl_data_source_listener ELinuxWindowWayland::kWlDataSourceListener = {
    .target = [](void* data,
                 wl_data_source* wl_data_source,
                 const char* mime_type) -> void {
      ELINUX_LOG(TRACE) << "wl_data_device_listener.target";
    },
    .send = [](void* data,
               wl_data_source* wl_data_source,
               const char* mime_type,
               int32_t fd) -> void {
      ELINUX_LOG(TRACE) << "wl_data_device_listener.send";

      if (strcmp(mime_type, kClipboardMimeTypeText)) {
        ELINUX_LOG(ERROR) << "Not expected mime_type: " << mime_type;
        return;
      }
      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      // Write the copied data to the clipboard.
      write(fd, self->clipboard_data_.c_str(),
            strlen(self->clipboard_data_.c_str()));
      close(fd);
    },
    .cancelled = [](void* data, wl_data_source* wl_data_source) -> void {
      ELINUX_LOG(TRACE) << "wl_data_device_listener.cancelled";

      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      self->clipboard_data_ = "";
      if (self->wl_data_source_) {
        wl_data_source_destroy(self->wl_data_source_);
        self->wl_data_source_ = nullptr;
      }
    },
    .dnd_drop_performed = [](void* data,
                             wl_data_source* wl_data_source) -> void {
      ELINUX_LOG(TRACE) << "wl_data_device_listener.dnd_drop_performed";
    },
    .dnd_finished = [](void* data, wl_data_source* wl_data_source) -> void {
      ELINUX_LOG(TRACE) << "wl_data_device_listener.dnd_finished";
    },
    .action = [](void* data,
                 wl_data_source* wl_data_source,
                 uint32_t dnd_action) -> void {
      ELINUX_LOG(TRACE) << "wl_data_device_listener.action";
    },
};

const zxdg_toplevel_decoration_v1_listener
    ELinuxWindowWayland::kZxdgToplevelDecorationV1Listener = {
        .configure =
            [](void* data,
               struct zxdg_toplevel_decoration_v1* zxdg_toplevel_decoration_v1,
               uint32_t mode) -> void {
          ELINUX_LOG(INFO)
              << "zxdg_toplevel_decoration_v1_listener.configure: mode is "
              << ((mode == ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE)
                      ? "server-side"
                      : "client-side");
          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
          if (self->view_properties_.use_window_decoration &&
              mode == ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE &&
              !self->window_decorations_) {
            int32_t width_dip = self->view_properties_.width;
            int32_t height_dip = self->view_properties_.height;
            self->CreateDecoration(width_dip, height_dip);
          }
        },
};

ELinuxWindowWayland::ELinuxWindowWayland(
    FlutterDesktopViewProperties view_properties)
    : cursor_info_({"", 0, nullptr}),
      display_valid_(false),
      running_(false),
      maximised_(false),
      is_requested_show_virtual_keyboard_(false),
      xdg_toplevel_(nullptr),
      wl_compositor_(nullptr),
      wl_subcompositor_(nullptr),
      wl_current_surface_(nullptr),
      wl_seat_(nullptr),
      wl_pointer_(nullptr),
      wl_touch_(nullptr),
      wl_keyboard_(nullptr),
      wl_shm_(nullptr),
      wl_data_device_manager_(nullptr),
      wl_data_device_(nullptr),
      wl_data_offer_(nullptr),
      wl_data_source_(nullptr),
      serial_(0),
      zwp_text_input_manager_v1_(nullptr),
      zwp_text_input_manager_v3_(nullptr),
      zwp_text_input_v1_(nullptr),
      zwp_text_input_v3_(nullptr),
      wp_presentation_(nullptr),
      wp_presentation_clk_id_(UINT32_MAX),
      window_decorations_(nullptr) {
  view_properties_ = view_properties;
  current_scale_ =
      view_properties.force_scale_factor ? view_properties.scale_factor : 1.0;
  SetRotation(view_properties_.view_rotation);

  auto xcursor_size_string = std::getenv(kXcursorSizeEnvironmentKey);
  cursor_size_ =
      xcursor_size_string ? atoi(xcursor_size_string) : kDefaultPointerSize;
  if (cursor_size_ <= 0) {
    cursor_size_ = kDefaultPointerSize;
  }

  wl_display_ = wl_display_connect(nullptr);
  if (!wl_display_) {
    ELINUX_LOG(ERROR) << "Failed to connect to the Wayland display.";
    return;
  }

  wl_registry_ = wl_display_get_registry(wl_display_);
  if (!wl_registry_) {
    ELINUX_LOG(ERROR) << "Failed to get the wayland registry.";
    return;
  }

  wl_registry_add_listener(wl_registry_, &kWlRegistryListener, this);
  wl_display_roundtrip(wl_display_);

  if (wl_data_device_manager_ && wl_seat_) {
    wl_data_device_ = wl_data_device_manager_get_data_device(
        wl_data_device_manager_, wl_seat_);
    wl_data_device_add_listener(wl_data_device_, &kWlDataDeviceListener, this);
  }

  // Setup text-input protocol for onscreen keyboard inputs.
  {
    if (zwp_text_input_manager_v3_ && wl_seat_) {
      zwp_text_input_v3_ = zwp_text_input_manager_v3_get_text_input(
          zwp_text_input_manager_v3_, wl_seat_);
      if (!zwp_text_input_v3_) {
        ELINUX_LOG(ERROR) << "Failed to create the text input manager v3.";
        return;
      }
      zwp_text_input_v3_add_listener(zwp_text_input_v3_,
                                     &kZwpTextInputV3Listener, this);
    } else if (zwp_text_input_manager_v1_) {
      zwp_text_input_v1_ = zwp_text_input_manager_v1_create_text_input(
          zwp_text_input_manager_v1_);
      if (!zwp_text_input_v1_) {
        ELINUX_LOG(ERROR) << "Failed to create text input manager v1.";
        return;
      }
      zwp_text_input_v1_add_listener(zwp_text_input_v1_,
                                     &kZwpTextInputV1Listener, this);
    } else {
      // do nothing.
    }
  }

  display_valid_ = true;
  running_ = true;
}

ELinuxWindowWayland::~ELinuxWindowWayland() {
  display_valid_ = false;
  running_ = false;

  for (auto theme : wl_cursor_themes_) {
    wl_cursor_theme_destroy(theme.second);
  }
  wl_cursor_themes_.clear();

  {
    if (zwp_text_input_v1_) {
      zwp_text_input_v1_destroy(zwp_text_input_v1_);
      zwp_text_input_v1_ = nullptr;
    }

    if (zwp_text_input_manager_v1_) {
      zwp_text_input_manager_v1_destroy(zwp_text_input_manager_v1_);
      zwp_text_input_manager_v1_ = nullptr;
    }

    if (zwp_text_input_v3_) {
      zwp_text_input_v3_destroy(zwp_text_input_v3_);
      zwp_text_input_v3_ = nullptr;
    }

    if (zwp_text_input_manager_v3_) {
      zwp_text_input_manager_v3_destroy(zwp_text_input_manager_v3_);
      zwp_text_input_manager_v3_ = nullptr;
    }
  }

  {
    if (zxdg_decoration_manager_v1_) {
      zxdg_decoration_manager_v1_destroy(zxdg_decoration_manager_v1_);
      zxdg_decoration_manager_v1_ = nullptr;
    }

    if (zxdg_toplevel_decoration_v1_) {
      zxdg_toplevel_decoration_v1_destroy(zxdg_toplevel_decoration_v1_);
      zxdg_toplevel_decoration_v1_ = nullptr;
    }
  }

  if (wl_data_offer_) {
    wl_data_offer_destroy(wl_data_offer_);
    wl_data_offer_ = nullptr;
  }

  if (wl_data_source_) {
    wl_data_source_destroy(wl_data_source_);
    wl_data_source_ = nullptr;
  }

  if (wl_data_device_) {
    if (wl_data_device_manager_version_ >=
        WL_DATA_DEVICE_RELEASE_SINCE_VERSION) {
      wl_data_device_release(wl_data_device_);
    } else {
      wl_data_device_destroy(wl_data_device_);
    }
    wl_data_device_ = nullptr;
  }

  if (wl_data_device_manager_) {
    wl_data_device_manager_destroy(wl_data_device_manager_);
    wl_data_device_manager_ = nullptr;
  }

  if (wl_pointer_) {
    wl_pointer_destroy(wl_pointer_);
    wl_pointer_ = nullptr;
  }

  if (wl_touch_) {
    wl_touch_destroy(wl_touch_);
    wl_touch_ = nullptr;
  }

  if (wl_keyboard_) {
    wl_keyboard_destroy(wl_keyboard_);
    wl_keyboard_ = nullptr;
  }

  if (wl_seat_) {
    wl_seat_destroy(wl_seat_);
    wl_seat_ = nullptr;
  }

  if (wl_output_) {
    wl_output_destroy(wl_output_);
    wl_output_ = nullptr;
  }

  if (wl_shm_) {
    wl_shm_destroy(wl_shm_);
    wl_shm_ = nullptr;
  }

  if (xdg_toplevel_) {
    xdg_toplevel_destroy(xdg_toplevel_);
    xdg_toplevel_ = nullptr;
  }

  if (xdg_wm_base_) {
    xdg_wm_base_destroy(xdg_wm_base_);
    xdg_wm_base_ = nullptr;
  }

  if (wl_compositor_) {
    wl_compositor_destroy(wl_compositor_);
    wl_compositor_ = nullptr;
  }

  if (wl_subcompositor_) {
    wl_subcompositor_destroy(wl_subcompositor_);
    wl_subcompositor_ = nullptr;
  }

  if (wl_registry_) {
    wl_registry_destroy(wl_registry_);
    wl_registry_ = nullptr;
  }

  if (wl_display_) {
    wl_display_flush(wl_display_);
    wl_display_disconnect(wl_display_);
    wl_display_ = nullptr;
  }
}

void ELinuxWindowWayland::SetView(WindowBindingHandlerDelegate* window) {
  binding_handler_delegate_ = window;
}

ELinuxRenderSurfaceTarget* ELinuxWindowWayland::GetRenderSurfaceTarget() const {
  return render_surface_.get();
}

uint16_t ELinuxWindowWayland::GetRotationDegree() const {
  return current_rotation_;
}

double ELinuxWindowWayland::GetDpiScale() {
  return current_scale_;
}

PhysicalWindowBounds ELinuxWindowWayland::GetPhysicalWindowBounds() {
  return {GetCurrentWidth(), GetCurrentHeight()};
}

int32_t ELinuxWindowWayland::GetFrameRate() {
  return frame_rate_;
}

bool ELinuxWindowWayland::DispatchEvent() {
  if (!IsValid()) {
    ELINUX_LOG(ERROR) << "Wayland display is invalid.";
    return false;
  }

  if (!running_) {
    return false;
  }

  if (request_redraw_) {
    request_redraw_ = false;
    if (window_decorations_) {
      window_decorations_->Resize(view_properties_.width,
                                  view_properties_.height, current_scale_);
    }

    if (binding_handler_delegate_) {
      binding_handler_delegate_->OnWindowSizeChanged(
          view_properties_.width * current_scale_,
          view_properties_.height * current_scale_ -
              WindowDecorationsPhysicalHeight());
    }
    NotifyDisplayInfoUpdates();
  }

  // Prepare to call wl_display_read_events.
  while (wl_display_prepare_read(wl_display_) != 0) {
    // If Wayland compositor terminates, -1 is returned.
    auto result = wl_display_dispatch_pending(wl_display_);
    if (result == -1) {
      return false;
    }
  }
  wl_display_flush(wl_display_);

  // Handle Vsync.
  if (binding_handler_delegate_) {
    const uint64_t vsync_interval_time_nanos = 1000000000000 / frame_rate_;
    binding_handler_delegate_->OnVsync(last_frame_time_nanos_,
                                       vsync_interval_time_nanos);
  }

  // Handle Wayland events.
  pollfd fds[] = {
      {wl_display_get_fd(wl_display_), POLLIN},
  };
  if (poll(fds, 1, 0) > 0) {
    auto result = wl_display_read_events(wl_display_);
    if (result == -1) {
      return false;
    }

    result = wl_display_dispatch_pending(wl_display_);
    if (result == -1) {
      return false;
    }
  } else {
    wl_display_cancel_read(wl_display_);
  }

  return true;
}

bool ELinuxWindowWayland::CreateRenderSurface(int32_t width_px,
                                              int32_t height_px,
                                              bool enable_impeller) {
  enable_impeller_ = enable_impeller;

  if (!display_valid_) {
    ELINUX_LOG(ERROR) << "Wayland display is invalid.";
    return false;
  }

  if (!wl_compositor_) {
    ELINUX_LOG(ERROR) << "Wl_compositor is invalid";
    return false;
  }

  if (!xdg_wm_base_) {
    ELINUX_LOG(ERROR) << "Xdg-shell is invalid";
    return false;
  }

  if (view_properties_.view_mode == FlutterDesktopViewMode::kFullscreen) {
    width_px = view_properties_.width * current_scale_;
    height_px = view_properties_.height * current_scale_;
  }

  ELINUX_LOG(TRACE) << "Created the Wayland surface: " << width_px << "x"
                    << height_px;
  if (view_properties_.use_mouse_cursor) {
    wl_cursor_surface_ = wl_compositor_create_surface(wl_compositor_);
    if (!wl_cursor_surface_) {
      ELINUX_LOG(ERROR)
          << "Failed to create the compositor surface for cursor.";
      return false;
    }
  }

  if (current_rotation_ == 90 || current_rotation_ == 270) {
    std::swap(width_px, height_px);
  }
  native_window_ = std::make_unique<NativeWindowWayland>(
      wl_compositor_, width_px, height_px, view_properties_.enable_vsync);

  wl_surface_add_listener(native_window_->Surface(), &kWlSurfaceListener, this);

  xdg_surface_ =
      xdg_wm_base_get_xdg_surface(xdg_wm_base_, native_window_->Surface());
  if (!xdg_surface_) {
    ELINUX_LOG(ERROR) << "Failed to get the xdg surface.";
    return false;
  }
  xdg_surface_add_listener(xdg_surface_, &kXdgSurfaceListener, this);

  xdg_toplevel_ = xdg_surface_get_toplevel(xdg_surface_);
  if (view_properties_.title != nullptr) {
    xdg_toplevel_set_title(xdg_toplevel_, view_properties_.title);
  }
  if (view_properties_.app_id != nullptr) {
    xdg_toplevel_set_app_id(xdg_toplevel_, view_properties_.app_id);
  }
  xdg_toplevel_add_listener(xdg_toplevel_, &kXdgToplevelListener, this);
  wl_surface_set_buffer_scale(native_window_->Surface(), current_scale_);

  {
    auto* callback = wl_surface_frame(native_window_->Surface());
    wl_callback_add_listener(callback, &kWlSurfaceFrameListener, this);

    if (wp_presentation_) {
      wp_presentation_feedback_add_listener(
          ::wp_presentation_feedback(wp_presentation_,
                                     native_window_->Surface()),
          &kWpPresentationFeedbackListener, this);
    }
  }

  if (view_properties_.view_mode == FlutterDesktopViewMode::kFullscreen) {
    xdg_toplevel_set_fullscreen(xdg_toplevel_, NULL);
  }

  wait_for_configure_ = true;
  wl_surface_commit(native_window_->Surface());

  render_surface_ = std::make_unique<SurfaceGl>(std::make_unique<ContextEgl>(
      std::make_unique<EnvironmentEgl>(wl_display_), enable_impeller));
  render_surface_->SetNativeWindow(native_window_.get());

  if (view_properties_.use_window_decoration) {
    if (zxdg_decoration_manager_v1_) {
      ELINUX_LOG(INFO) << "Use server-side xdg-decoration mode";
      zxdg_toplevel_decoration_v1_ =
          zxdg_decoration_manager_v1_get_toplevel_decoration(
              zxdg_decoration_manager_v1_, xdg_toplevel_);
      zxdg_toplevel_decoration_v1_set_mode(
          zxdg_toplevel_decoration_v1_,
          ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
      zxdg_toplevel_decoration_v1_add_listener(
          zxdg_toplevel_decoration_v1_, &kZxdgToplevelDecorationV1Listener,
          this);
    } else {
      int32_t width_dip = width_px / current_scale_;
      int32_t height_dip = height_px / current_scale_;
      CreateDecoration(width_dip, height_dip);
    }
  }

  // Wait for making sure that xdg_surface has been configured.
  while (wait_for_configure_) {
    wl_display_dispatch(wl_display_);
  }

  return true;
}

void ELinuxWindowWayland::CreateDecoration(int32_t width_dip,
                                           int32_t height_dip) {
  if (window_decorations_) {
    ELINUX_LOG(WARNING) << "Window decoration has already created";
    return;
  }

  window_decorations_ = std::make_unique<WindowDecorationsWayland>(
      wl_display_, wl_compositor_, wl_subcompositor_, native_window_->Surface(),
      width_dip, height_dip, current_scale_, enable_impeller_,
      view_properties_.enable_vsync);
}

void ELinuxWindowWayland::DestroyRenderSurface() {
  // destroy the main surface before destroying the client window on Wayland.
  if (window_decorations_) {
    window_decorations_ = nullptr;
  }
  render_surface_ = nullptr;
  native_window_ = nullptr;

  if (xdg_surface_) {
    xdg_surface_destroy(xdg_surface_);
    xdg_surface_ = nullptr;
  }

  if (wl_cursor_surface_) {
    wl_surface_destroy(wl_cursor_surface_);
    wl_cursor_surface_ = nullptr;
  }
}

void ELinuxWindowWayland::UpdateVirtualKeyboardStatus(const bool show) {
  // Not supported virtual keyboard.
  if (!(zwp_text_input_v1_ || zwp_text_input_v3_) || !wl_seat_) {
    return;
  }

  is_requested_show_virtual_keyboard_ = show;
  if (is_requested_show_virtual_keyboard_) {
    ShowVirtualKeyboard();
  } else {
    DismissVirtualKeybaord();
  }
}

void ELinuxWindowWayland::UpdateFlutterCursor(const std::string& cursor_name) {
  if (view_properties_.use_mouse_cursor) {
    if (cursor_name.compare(cursor_info_.cursor_name) == 0) {
      return;
    }
    cursor_info_.cursor_name = cursor_name;

    if (cursor_name.compare(kCursorNameNone) == 0) {
      // Turn off the cursor.
      wl_pointer_set_cursor(cursor_info_.pointer, cursor_info_.serial,
                            wl_cursor_surface_, 0, 0);
      wl_surface_attach(wl_cursor_surface_, nullptr, 0, 0);
      wl_surface_damage(wl_cursor_surface_, 0, 0, 0, 0);
      wl_surface_commit(wl_cursor_surface_);
      return;
    }

    auto wl_cursor = GetWlCursor(cursor_name, cursor_size_ * current_scale_);
    if (!wl_cursor) {
      return;
    }
    auto image = wl_cursor->images[0];
    auto buffer = wl_cursor_image_get_buffer(image);
    if (buffer) {
      wl_pointer_set_cursor(
          cursor_info_.pointer, cursor_info_.serial, wl_cursor_surface_,
          image->hotspot_x / current_scale_, image->hotspot_y / current_scale_);
      wl_surface_attach(wl_cursor_surface_, buffer, 0, 0);
      wl_surface_damage(wl_cursor_surface_, 0, 0, image->width, image->height);
      wl_surface_set_buffer_scale(wl_cursor_surface_, current_scale_);
      wl_surface_commit(wl_cursor_surface_);
    }
  }
}

std::string ELinuxWindowWayland::GetClipboardData() {
  std::string str = "";

  if (wl_data_offer_) {
    int fd[2];
    if (pipe2(fd, O_CLOEXEC) == -1) {
      return str;
    }

    wl_data_offer_receive(wl_data_offer_, kClipboardMimeTypeText, fd[1]);
    close(fd[1]);
    wl_display_dispatch(wl_display_);

    char buf[256];
    int len;
    // Read data form the clipboard.
    while ((len = read(fd[0], buf, sizeof(buf))) > 0) {
      str.append(buf, len);
    }
    close(fd[0]);
    return str;
  }

  return str;
}

void ELinuxWindowWayland::SetClipboardData(const std::string& data) {
  clipboard_data_ = data;
  if (wl_data_device_manager_) {
    if (wl_data_source_) {
      wl_data_source_destroy(wl_data_source_);
      wl_data_source_ = nullptr;
    }

    wl_data_source_ =
        wl_data_device_manager_create_data_source(wl_data_device_manager_);
    if (!wl_data_source_) {
      return;
    }

    wl_data_source_offer(wl_data_source_, kClipboardMimeTypeText);
    wl_data_source_add_listener(wl_data_source_, &kWlDataSourceListener, this);
    wl_data_device_set_selection(wl_data_device_, wl_data_source_, serial_);
  }
}

bool ELinuxWindowWayland::IsValid() const {
  if (!display_valid_ || !native_window_ || !render_surface_ ||
      !native_window_->IsValid() || !render_surface_->IsValid()) {
    return false;
  }
  return true;
}

void ELinuxWindowWayland::WlRegistryHandler(wl_registry* wl_registry,
                                            uint32_t name,
                                            const char* interface,
                                            uint32_t version) {
  if (!strcmp(interface, wl_compositor_interface.name)) {
    constexpr uint32_t kMaxVersion = 5;
    wl_compositor_ = static_cast<decltype(wl_compositor_)>(
        wl_registry_bind(wl_registry, name, &wl_compositor_interface,
                         std::min(kMaxVersion, version)));
    return;
  }

  if (!strcmp(interface, wl_subcompositor_interface.name)) {
    constexpr uint32_t kMaxVersion = 1;
    wl_subcompositor_ = static_cast<wl_subcompositor*>(wl_registry_bind(
        wl_registry, name, &wl_subcompositor_interface, kMaxVersion));
  }

  if (!strcmp(interface, xdg_wm_base_interface.name)) {
    constexpr uint32_t kMaxVersion = 3;
    xdg_wm_base_ = static_cast<decltype(xdg_wm_base_)>(
        wl_registry_bind(wl_registry, name, &xdg_wm_base_interface,
                         std::min(kMaxVersion, version)));
    xdg_wm_base_add_listener(xdg_wm_base_, &kXdgWmBaseListener, this);
    return;
  }

  if (!strcmp(interface, wl_seat_interface.name)) {
    constexpr uint32_t kMaxVersion = 4;
    wl_seat_ = static_cast<decltype(wl_seat_)>(wl_registry_bind(
        wl_registry, name, &wl_seat_interface, std::min(kMaxVersion, version)));
    wl_seat_add_listener(wl_seat_, &kWlSeatListener, this);
    return;
  }

  if (!strcmp(interface, wl_output_interface.name)) {
    constexpr uint32_t kMaxVersion = 2;
    wl_output_ = static_cast<decltype(wl_output_)>(
        wl_registry_bind(wl_registry, name, &wl_output_interface,
                         std::min(kMaxVersion, version)));
    wl_output_add_listener(wl_output_, &kWlOutputListener, this);
    return;
  }

  if (!strcmp(interface, wl_shm_interface.name)) {
    if (view_properties_.use_mouse_cursor) {
      constexpr uint32_t kMaxVersion = 1;
      wl_shm_ = static_cast<decltype(wl_shm_)>(
          wl_registry_bind(wl_registry, name, &wl_shm_interface, kMaxVersion));
    }
    return;
  }

  if (!strcmp(interface, kZwpTextInputManagerV1)) {
    if (view_properties_.use_onscreen_keyboard) {
      constexpr uint32_t kMaxVersion = 1;
      zwp_text_input_manager_v1_ =
          static_cast<decltype(zwp_text_input_manager_v1_)>(wl_registry_bind(
              wl_registry, name, &zwp_text_input_manager_v1_interface,
              kMaxVersion));
    }
    return;
  }

  if (!strcmp(interface, kZwpTextInputManagerV3)) {
    if (view_properties_.use_onscreen_keyboard) {
      constexpr uint32_t kMaxVersion = 1;
      zwp_text_input_manager_v3_ =
          static_cast<decltype(zwp_text_input_manager_v3_)>(wl_registry_bind(
              wl_registry, name, &zwp_text_input_manager_v3_interface,
              std::min(kMaxVersion, version)));
    }
    return;
  }

  if (!strcmp(interface, wl_data_device_manager_interface.name)) {
    // Save the version of wl_data_device_manager because the release method of
    // wl_data_device differs depending on it. Since wl_data_device_manager has
    // been released up to version 3, set the upper limit to 3.
    constexpr uint32_t kMaxVersion = 3;
    wl_data_device_manager_version_ = std::min(kMaxVersion, version);
    wl_data_device_manager_ = static_cast<decltype(wl_data_device_manager_)>(
        wl_registry_bind(wl_registry, name, &wl_data_device_manager_interface,
                         wl_data_device_manager_version_));
    return;
  }

  if (!strcmp(interface, wp_presentation_interface.name)) {
    constexpr uint32_t kMaxVersion = 1;
    wp_presentation_ = static_cast<decltype(wp_presentation_)>(wl_registry_bind(
        wl_registry, name, &wp_presentation_interface, kMaxVersion));
    wp_presentation_add_listener(wp_presentation_, &kWpPresentationListener,
                                 this);
    return;
  }

  if (!strcmp(interface, kZxdgDecorationManagerV1)) {
    constexpr uint32_t kMaxVersion = 1;
    zxdg_decoration_manager_v1_ =
        static_cast<decltype(zxdg_decoration_manager_v1_)>(wl_registry_bind(
            wl_registry, name, &zxdg_decoration_manager_v1_interface,
            std::min(kMaxVersion, version)));
    return;
  }
}

void ELinuxWindowWayland::WlUnRegistryHandler(wl_registry* wl_registry,
                                              uint32_t name) {}

bool ELinuxWindowWayland::LoadCursorTheme(uint32_t size) {
  if (!wl_shm_) {
    ELINUX_LOG(ERROR) << "Failed to load cursor theme because shared memory "
                         "buffers are not available.";
    return false;
  }

  auto theme = wl_cursor_theme_load(nullptr, size, wl_shm_);
  if (!theme) {
    ELINUX_LOG(ERROR) << "Failed to load cursor theme for size: " << size;
    return false;
  }

  wl_cursor_themes_[size] = theme;

  std::vector<std::string> wl_cursor_names{
      kWlCursorThemeLeftPtr,
      kWlCursorThemeBottomLeftCorner,
      kWlCursorThemeBottomRightCorner,
      kWlCursorThemeBottomSide,
      kWlCursorThemeGrabbing,
      kWlCursorThemeLeftSide,
      kWlCursorThemeRightSide,
      kWlCursorThemeTopLeftCorner,
      kWlCursorThemeTopRightCorner,
      kWlCursorThemeTopSide,
      kWlCursorThemeXterm,
      kWlCursorThemeHand1,
      kWlCursorThemeWatch,
  };

  std::unordered_map<std::string, wl_cursor*> cursor_list;

  for (const auto& cursor_name : wl_cursor_names) {
    auto wl_cursor = wl_cursor_theme_get_cursor(theme, cursor_name.c_str());
    if (!wl_cursor) {
      ELINUX_LOG(ERROR) << "Unsupported cursor theme: " << cursor_name.c_str();
      continue;
    }
    cursor_list[cursor_name] = wl_cursor;
  }

  supported_wl_cursor_list_.insert(std::make_pair(size, cursor_list));

  return true;
}

wl_cursor* ELinuxWindowWayland::GetWlCursor(const std::string& cursor_name,
                                            uint32_t size) {
  // Convert the cursor theme name from Flutter's cursor value to Wayland's one.
  // However, Wayland has not all cursor themes corresponding to Flutter.
  // If there is no Wayland's cursor theme corresponding to the Flutter's cursor
  // name, it is defined as empty.
  // If empty, the default cursor theme(left_ptr) will be displayed.
  static const std::unordered_map<std::string, std::string>
      flutter_to_wayland_cursor_map = {
          {"alias", ""},
          {"allScroll", ""},
          {"basic", kWlCursorThemeLeftPtr},
          {"cell", ""},
          {"click", kWlCursorThemeHand1},
          {"contextMenu", ""},
          {"copy", ""},
          {"forbidden", ""},
          {"grab", ""},
          {"grabbing", kWlCursorThemeGrabbing},
          {"help", ""},
          {"move", ""},
          {"noDrop", ""},
          {"precise", ""},
          {"progress", ""},
          {"text", kWlCursorThemeXterm},
          {"resizeColumn", ""},
          {"resizeDown", kWlCursorThemeBottomSide},
          {"resizeDownLeft", kWlCursorThemeBottomLeftCorner},
          {"resizeDownRight", kWlCursorThemeBottomRightCorner},
          {"resizeLeft", kWlCursorThemeLeftSide},
          {"resizeLeftRight", ""},
          {"resizeRight", kWlCursorThemeRightSide},
          {"resizeRow", ""},
          {"resizeUp", kWlCursorThemeTopSide},
          {"resizeUpDown", ""},
          {"resizeUpLeft", kWlCursorThemeTopLeftCorner},
          {"resizeUpRight", kWlCursorThemeTopRightCorner},
          {"resizeUpLeftDownRight", ""},
          {"resizeUpRightDownLeft", ""},
          {"verticalText", ""},
          {"wait", kWlCursorThemeWatch},
          {"zoomIn", ""},
          {"zoomOut", ""},
      };

  if (supported_wl_cursor_list_.find(size) == supported_wl_cursor_list_.end()) {
    if (!LoadCursorTheme(size)) {
      return nullptr;
    }
  }

  auto cursor_list = supported_wl_cursor_list_.at(size);

  if (flutter_to_wayland_cursor_map.find(cursor_name) !=
      flutter_to_wayland_cursor_map.end()) {
    auto theme = flutter_to_wayland_cursor_map.at(cursor_name);
    if (!theme.empty() && cursor_list.find(theme) != cursor_list.end()) {
      return cursor_list[theme];
    }
  }

  ELINUX_LOG(ERROR) << "Unsupported cursor: " << cursor_name.c_str();
  return cursor_list[kWlCursorThemeLeftPtr];
}

void ELinuxWindowWayland::ShowVirtualKeyboard() {
  if (zwp_text_input_v3_) {
    // I'm not sure the reason, but enable needs to be called twice.
    zwp_text_input_v3_enable(zwp_text_input_v3_);
    zwp_text_input_v3_commit(zwp_text_input_v3_);
    zwp_text_input_v3_enable(zwp_text_input_v3_);
    zwp_text_input_v3_commit(zwp_text_input_v3_);

    zwp_text_input_v3_set_content_type(
        zwp_text_input_v3_, ZWP_TEXT_INPUT_V3_CONTENT_HINT_NONE,
        ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_TERMINAL);
    zwp_text_input_v3_commit(zwp_text_input_v3_);
  } else {
    if (native_window_) {
      zwp_text_input_v1_show_input_panel(zwp_text_input_v1_);
      zwp_text_input_v1_activate(zwp_text_input_v1_, wl_seat_,
                                 native_window_->Surface());
    }
  }
}

void ELinuxWindowWayland::DismissVirtualKeybaord() {
  if (zwp_text_input_v3_) {
    zwp_text_input_v3_disable(zwp_text_input_v3_);
    zwp_text_input_v3_commit(zwp_text_input_v3_);
  } else {
    zwp_text_input_v1_deactivate(zwp_text_input_v1_, wl_seat_);
  }
}

void ELinuxWindowWayland::UpdateWindowScale() {
  if (view_properties_.force_scale_factor) {
    return;
  }

  double scale_factor = 1.0;
  for (auto output_id : entered_outputs_) {
    if (wl_output_scale_factors_.find(output_id) ==
        wl_output_scale_factors_.end())
      continue;

    auto output_scale_factor = wl_output_scale_factors_.at(output_id);
    if (output_scale_factor > scale_factor)
      scale_factor = output_scale_factor;
  }

  if (current_scale_ == scale_factor) {
    return;
  }

  ELINUX_LOG(TRACE) << "Window scale has changed: " << scale_factor;
  current_scale_ = scale_factor;

  wl_surface_set_buffer_scale(native_window_->Surface(), current_scale_);
  request_redraw_ = true;
}

uint32_t ELinuxWindowWayland::WindowDecorationsPhysicalHeight() const {
  if (!window_decorations_) {
    return 0;
  }

  return window_decorations_->Height() * current_scale_;
}

}  // namespace flutter
