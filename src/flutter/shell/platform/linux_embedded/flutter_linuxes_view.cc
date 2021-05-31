// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/flutter_linuxes_view.h"

#include <chrono>

#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

FlutterLinuxesView::FlutterLinuxesView(
    std::unique_ptr<WindowBindingHandler> window_binding) {
  // Take the binding handler, and give it a pointer back to self.
  binding_handler_ = std::move(window_binding);
  binding_handler_->SetView(this);
}

FlutterLinuxesView::~FlutterLinuxesView() {
  // Need to stop running the Engine before destroying surface.
  if (engine_) {
    engine_->Stop();
  }
  DestroyRenderSurface();
}

bool FlutterLinuxesView::DispatchEvent() {
  return binding_handler_->DispatchEvent();
}

void FlutterLinuxesView::SetEngine(
    std::unique_ptr<FlutterLinuxesEngine> engine) {
  engine_ = std::move(engine);

  engine_->SetView(this);

  internal_plugin_registrar_ =
      std::make_unique<flutter::PluginRegistrar>(engine_->GetRegistrar());

  // Set up the system channel handlers.
  auto internal_plugin_messenger = internal_plugin_registrar_->messenger();
  keyboard_handler_ =
      std::make_unique<flutter::KeyeventPlugin>(internal_plugin_messenger);
  textinput_handler_ = std::make_unique<flutter::TextInputPlugin>(
      internal_plugin_messenger, binding_handler_.get());
  platform_handler_ = std::make_unique<flutter::PlatformPlugin>(
      internal_plugin_messenger, binding_handler_.get());
  cursor_handler_ = std::make_unique<flutter::MouseCursorPlugin>(
      internal_plugin_messenger, binding_handler_.get());
  lifecycle_handler_ =
      std::make_unique<flutter::LifecyclePlugin>(internal_plugin_messenger);

  PhysicalWindowBounds bounds = binding_handler_->GetPhysicalWindowBounds();
  SendWindowMetrics(bounds.width, bounds.height,
                    binding_handler_->GetDpiScale());
}

void FlutterLinuxesView::OnWindowSizeChanged(size_t width,
                                             size_t height) const {
  if (!GetRenderSurfaceTarget()->OnScreenSurfaceResize(width, height)) {
    LINUXES_LOG(ERROR) << "Failed to change surface size.";
    return;
  }
  SendWindowMetrics(width, height, binding_handler_->GetDpiScale());
}

void FlutterLinuxesView::OnPointerMove(double x, double y) {
  SendPointerMove(x, y);
}

void FlutterLinuxesView::OnPointerDown(
    double x, double y, FlutterPointerMouseButtons flutter_button) {
  if (flutter_button != 0) {
    uint64_t mouse_buttons = mouse_state_.buttons | flutter_button;
    SetMouseButtons(mouse_buttons);
    SendPointerDown(x, y);
  }
}

void FlutterLinuxesView::OnPointerUp(
    double x, double y, FlutterPointerMouseButtons flutter_button) {
  if (flutter_button != 0) {
    uint64_t mouse_buttons = mouse_state_.buttons & ~flutter_button;
    SetMouseButtons(mouse_buttons);
    SendPointerUp(x, y);
  }
}

void FlutterLinuxesView::OnPointerLeave() { SendPointerLeave(); }

void FlutterLinuxesView::OnTouchDown(uint32_t time, int32_t id, double x,
                                     double y) {
  auto* point = GgeTouchPoint(id);
  if (!point) {
    return;
  }
  point->event_mask = TouchEvent::kDown;
  point->x = x;
  point->y = y;
  touch_event_.time = time;

  FlutterPointerEvent event = {
      .struct_size = sizeof(event),
      .phase = FlutterPointerPhase::kDown,
      .timestamp = time,
      .x = x,
      .y = y,
      .device = id,
      .signal_kind = kFlutterPointerSignalKindNone,
      .scroll_delta_x = 0,
      .scroll_delta_y = 0,
      .device_kind = kFlutterPointerDeviceKindTouch,
      .buttons = 0,
  };
  engine_->SendPointerEvent(event);
}

void FlutterLinuxesView::OnTouchUp(uint32_t time, int32_t id) {
  auto* point = GgeTouchPoint(id);
  if (!point) {
    return;
  }
  point->event_mask = TouchEvent::kUp;

  FlutterPointerEvent event = {
      .struct_size = sizeof(event),
      .phase = FlutterPointerPhase::kUp,
      .timestamp = time,
      .x = point->x,
      .y = point->y,
      .device = id,
      .signal_kind = kFlutterPointerSignalKindNone,
      .scroll_delta_x = 0,
      .scroll_delta_y = 0,
      .device_kind = kFlutterPointerDeviceKindTouch,
      .buttons = 0,
  };
  engine_->SendPointerEvent(event);
}

void FlutterLinuxesView::OnTouchMotion(uint32_t time, int32_t id, double x,
                                       double y) {
  auto* point = GgeTouchPoint(id);
  if (!point) {
    return;
  }
  point->event_mask = TouchEvent::kMotion;
  point->x = x;
  point->y = y;
  touch_event_.time = time;

  FlutterPointerEvent event = {
      .struct_size = sizeof(event),
      .phase = FlutterPointerPhase::kMove,
      .timestamp = time,
      .x = x,
      .y = y,
      .device = id,
      .signal_kind = kFlutterPointerSignalKindNone,
      .scroll_delta_x = 0,
      .scroll_delta_y = 0,
      .device_kind = kFlutterPointerDeviceKindTouch,
      .buttons = 0,
  };
  engine_->SendPointerEvent(event);
}

void FlutterLinuxesView::OnTouchCancel() {}

void FlutterLinuxesView::OnKeyMap(uint32_t format, int fd, uint32_t size) {
  keyboard_handler_->OnKeymap(format, fd, size);
}

void FlutterLinuxesView::OnKey(uint32_t key, bool pressed) {
  keyboard_handler_->OnKey(key, pressed);
  if (pressed) {
    auto code_point = keyboard_handler_->GetCodePoint(key);
    if (!keyboard_handler_->IsTextInputSuppressed(code_point)) {
      textinput_handler_->OnKeyPressed(key, code_point);
    }
  }
}

void FlutterLinuxesView::OnKeyModifiers(uint32_t mods_depressed,
                                        uint32_t mods_latched,
                                        uint32_t mods_locked, uint32_t group) {
  keyboard_handler_->OnModifiers(mods_depressed, mods_latched, mods_locked,
                                 group);
}

void FlutterLinuxesView::OnVirtualKey(uint32_t code_point) {
  // Since the keycode cannot be specified, set an invalid value(0).
  constexpr uint32_t kCharKey = 0;
  textinput_handler_->OnKeyPressed(kCharKey, code_point);
}

void FlutterLinuxesView::OnVirtualSpecialKey(uint32_t keycode) {
  auto code_point = keyboard_handler_->GetCodePoint(keycode);
  textinput_handler_->OnKeyPressed(keycode, code_point);
}

void FlutterLinuxesView::OnScroll(double x, double y, double delta_x,
                                  double delta_y,
                                  int scroll_offset_multiplier) {
  SendScroll(x, y, delta_x, delta_y, scroll_offset_multiplier);
}

void FlutterLinuxesView::OnVsync(uint64_t last_frame_time_nanos,
                                 uint64_t vsync_interval_time_nanos) {
  engine_->OnVsync(last_frame_time_nanos, vsync_interval_time_nanos);
}

FlutterLinuxesView::touch_point* FlutterLinuxesView::GgeTouchPoint(int32_t id) {
  const size_t nmemb = sizeof(touch_event_) / sizeof(struct touch_point);
  int invalid = -1;
  for (size_t i = 0; i < nmemb; ++i) {
    if (touch_event_.points[i].id == id) {
      return &touch_event_.points[i];
    }
    if (invalid == -1 && !touch_event_.points[i].valid) {
      invalid = i;
    }
  }
  if (invalid == -1) {
    return nullptr;
  }
  touch_event_.points[invalid].valid = true;
  touch_event_.points[invalid].id = id;
  return &touch_event_.points[invalid];
}

// Sends new size  information to FlutterEngine.
void FlutterLinuxesView::SendWindowMetrics(size_t width, size_t height,
                                           double dpiScale) const {
  FlutterWindowMetricsEvent event = {};
  event.struct_size = sizeof(event);
  event.width = width;
  event.height = height;
  event.pixel_ratio = dpiScale;
  engine_->SendWindowMetricsEvent(event);
}

void FlutterLinuxesView::SendInitialBounds() {
  PhysicalWindowBounds bounds = binding_handler_->GetPhysicalWindowBounds();
  SendWindowMetrics(bounds.width, bounds.height,
                    binding_handler_->GetDpiScale());
}

// Set's |event_data|'s phase to either kMove or kHover depending on the current
// primary mouse button state.
void FlutterLinuxesView::SetEventPhaseFromCursorButtonState(
    FlutterPointerEvent* event_data) const {
  // For details about this logic, see FlutterPointerPhase in the embedder.h
  // file.
  event_data->phase =
      mouse_state_.buttons == 0
          ? mouse_state_.flutter_state_is_down ? FlutterPointerPhase::kUp
                                               : FlutterPointerPhase::kHover
          : mouse_state_.flutter_state_is_down ? FlutterPointerPhase::kMove
                                               : FlutterPointerPhase::kDown;
}

void FlutterLinuxesView::SendPointerMove(double x, double y) {
  FlutterPointerEvent event = {};
  event.x = x;
  event.y = y;
  SetEventPhaseFromCursorButtonState(&event);
  SendPointerEventWithData(event);
}

void FlutterLinuxesView::SendPointerDown(double x, double y) {
  FlutterPointerEvent event = {};
  SetEventPhaseFromCursorButtonState(&event);
  event.x = x;
  event.y = y;
  SendPointerEventWithData(event);
  SetMouseFlutterStateDown(true);
}

void FlutterLinuxesView::SendPointerUp(double x, double y) {
  FlutterPointerEvent event = {};
  SetEventPhaseFromCursorButtonState(&event);
  event.x = x;
  event.y = y;
  SendPointerEventWithData(event);
  if (event.phase == FlutterPointerPhase::kUp) {
    SetMouseFlutterStateDown(false);
  }
}

void FlutterLinuxesView::SendPointerLeave() {
  FlutterPointerEvent event = {};
  event.phase = FlutterPointerPhase::kRemove;
  SendPointerEventWithData(event);
}

void FlutterLinuxesView::SendScroll(double x, double y, double delta_x,
                                    double delta_y,
                                    int scroll_offset_multiplier) {
  FlutterPointerEvent event = {};
  SetEventPhaseFromCursorButtonState(&event);
  event.signal_kind = FlutterPointerSignalKind::kFlutterPointerSignalKindScroll;
  event.x = x;
  event.y = y;
  event.scroll_delta_x = delta_x * scroll_offset_multiplier;
  event.scroll_delta_y = delta_y * scroll_offset_multiplier;
  SendPointerEventWithData(event);
}

void FlutterLinuxesView::SendPointerEventWithData(
    const FlutterPointerEvent& event_data) {
  // If sending anything other than an add, and the pointer isn't already added,
  // synthesize an add to satisfy Flutter's expectations about events.
  if (!mouse_state_.flutter_state_is_added &&
      event_data.phase != FlutterPointerPhase::kAdd) {
    FlutterPointerEvent event = {};
    event.phase = FlutterPointerPhase::kAdd;
    event.x = event_data.x;
    event.y = event_data.y;
    event.buttons = 0;
    SendPointerEventWithData(event);
  }
  // Don't double-add (e.g., if events are delivered out of order, so an add has
  // already been synthesized).
  if (mouse_state_.flutter_state_is_added &&
      event_data.phase == FlutterPointerPhase::kAdd) {
    return;
  }

  FlutterPointerEvent event = event_data;
  event.device_kind = kFlutterPointerDeviceKindMouse;
  event.buttons = mouse_state_.buttons;

  // Set metadata that's always the same regardless of the event.
  event.struct_size = sizeof(event);
  event.timestamp =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();

  engine_->SendPointerEvent(event);

  if (event_data.phase == FlutterPointerPhase::kAdd) {
    SetMouseFlutterStateAdded(true);
  } else if (event_data.phase == FlutterPointerPhase::kRemove) {
    SetMouseFlutterStateAdded(false);
    ResetMouseState();
  }
}

void* FlutterLinuxesView::ProcResolver(const char* name) {
  return GetRenderSurfaceTarget()->GlProcResolver(name);
}

bool FlutterLinuxesView::MakeCurrent() {
  return GetRenderSurfaceTarget()->GLContextMakeCurrent();
}

bool FlutterLinuxesView::ClearCurrent() {
  return GetRenderSurfaceTarget()->GLContextClearCurrent();
}

bool FlutterLinuxesView::Present() {
  return GetRenderSurfaceTarget()->GLContextPresent(0);
}

uint32_t FlutterLinuxesView::GetOnscreenFBO() {
  return GetRenderSurfaceTarget()->GLContextFBO();
}

bool FlutterLinuxesView::MakeResourceCurrent() {
  return GetRenderSurfaceTarget()->ResourceContextMakeCurrent();
}

bool FlutterLinuxesView::CreateRenderSurface() {
  PhysicalWindowBounds bounds = binding_handler_->GetPhysicalWindowBounds();
  return binding_handler_->CreateRenderSurface(bounds.width, bounds.height);
}

void FlutterLinuxesView::DestroyRenderSurface() {
  binding_handler_->DestroyRenderSurface();
}

LinuxesRenderSurfaceTarget* FlutterLinuxesView::GetRenderSurfaceTarget() const {
  return binding_handler_->GetRenderSurfaceTarget();
}

FlutterLinuxesEngine* FlutterLinuxesView::GetEngine() { return engine_.get(); }

int32_t FlutterLinuxesView::GetFrameRate() {
  return binding_handler_->GetFrameRate();
}

}  // namespace flutter
