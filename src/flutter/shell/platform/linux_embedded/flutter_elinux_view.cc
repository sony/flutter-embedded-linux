// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/flutter_elinux_view.h"

#include <chrono>
#include <cmath>

#include "flutter/shell/platform/linux_embedded/logger.h"

namespace flutter {

namespace {
constexpr int kMicrosecondsPerMillisecond = 1000;

inline FlutterTransformation FlutterTransformationMake(const uint16_t& degree) {
  double radian = degree * M_PI / 180.0;
  FlutterTransformation transformation = {};
  transformation.scaleX = cos(radian);
  transformation.skewX = -sin(radian);
  transformation.transX = 0;
  transformation.skewY = sin(radian);
  transformation.scaleY = cos(radian);
  transformation.transY = 0;
  transformation.pers0 = 0;
  transformation.pers1 = 0;
  transformation.pers2 = 1;
  return transformation;
}

}  // namespace

FlutterELinuxView::FlutterELinuxView(
    std::unique_ptr<WindowBindingHandler> window_binding) {
  // Take the binding handler, and give it a pointer back to self.
  binding_handler_ = std::move(window_binding);
  binding_handler_->SetView(this);
}

FlutterELinuxView::~FlutterELinuxView() {
  // Need to stop running the Engine before destroying surface.
  if (engine_) {
    engine_->Stop();
  }
  DestroyRenderSurface();
}

bool FlutterELinuxView::DispatchEvent() {
  return binding_handler_->DispatchEvent();
}

void FlutterELinuxView::SetEngine(std::unique_ptr<FlutterELinuxEngine> engine) {
  engine_ = std::move(engine);

  engine_->SetView(this);

  internal_plugin_registrar_ =
      std::make_unique<flutter::PluginRegistrar>(engine_->GetRegistrar());

  // Set up the system channel handlers.
  auto internal_plugin_messenger = internal_plugin_registrar_->messenger();

  // Set up internal channels.
  // TODO: Replace this with an embedder.h API. See
  // https://github.com/flutter/flutter/issues/71099
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
  navigation_handler_ =
      std::make_unique<flutter::NavigationPlugin>(internal_plugin_messenger);
  platform_views_handler_ =
      std::make_unique<flutter::PlatformViewsPlugin>(internal_plugin_messenger);
  settings_handler_ = std::make_unique<flutter::SettingsPlugin>(
      internal_plugin_messenger, binding_handler_.get());

  PhysicalWindowBounds bounds = binding_handler_->GetPhysicalWindowBounds();
  SendWindowMetrics(bounds.width, bounds.height,
                    binding_handler_->GetDpiScale());
}

void FlutterELinuxView::RegisterPlatformViewFactory(
    const char* view_type,
    std::unique_ptr<FlutterDesktopPlatformViewFactory> factory) {
  platform_views_handler_->RegisterViewFactory(view_type, std::move(factory));
}

void FlutterELinuxView::OnWindowSizeChanged(size_t width_px,
                                            size_t height_px) const {
  if (!GetRenderSurfaceTarget()->OnScreenSurfaceResize(width_px, height_px)) {
    ELINUX_LOG(ERROR) << "Failed to change surface size.";
    return;
  }
  SendWindowMetrics(width_px, height_px, binding_handler_->GetDpiScale());
}

void FlutterELinuxView::OnPointerMove(double x_px, double y_px) {
  auto trimmed_xy = GetPointerRotation(x_px, y_px);
  SendPointerMove(trimmed_xy.first, trimmed_xy.second);
}

void FlutterELinuxView::OnPointerDown(
    double x_px,
    double y_px,
    FlutterPointerMouseButtons flutter_button) {
  if (flutter_button != 0) {
    uint64_t mouse_buttons = mouse_state_.buttons | flutter_button;
    auto trimmed_xy = GetPointerRotation(x_px, y_px);
    SetMouseButtons(mouse_buttons);
    SendPointerDown(trimmed_xy.first, trimmed_xy.second);
  }
}

void FlutterELinuxView::OnPointerUp(double x_px,
                                    double y_px,
                                    FlutterPointerMouseButtons flutter_button) {
  if (flutter_button != 0) {
    auto trimmed_xy = GetPointerRotation(x_px, y_px);
    uint64_t mouse_buttons = mouse_state_.buttons & ~flutter_button;
    SetMouseButtons(mouse_buttons);
    SendPointerUp(trimmed_xy.first, trimmed_xy.second);
  }
}

void FlutterELinuxView::OnPointerLeave() {
  SendPointerLeave();
}

void FlutterELinuxView::OnTouchDown(uint32_t time,
                                    int32_t id,
                                    double x,
                                    double y) {
  // Increase device-id to avoid
  // "FML_DCHECK(states_.find(pointer_data.device) == states_.end());"
  // exception in flutter/engine.
  // This is because "device-id = 0" is used for mouse inputs.
  // See engine/lib/ui/window/pointer_data_packet_converter.cc
  id += 1;

  auto trimmed_xy = GetPointerRotation(x, y);
  auto* point = GgeTouchPoint(id);
  if (!point) {
    return;
  }
  point->event_mask = TouchEvent::kDown;
  point->x = trimmed_xy.first;
  point->y = trimmed_xy.second;

  FlutterPointerEvent event = {
      .struct_size = sizeof(event),
      .phase = FlutterPointerPhase::kDown,
      .timestamp = time * kMicrosecondsPerMillisecond,
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

void FlutterELinuxView::OnTouchUp(uint32_t time, int32_t id) {
  // Increase device-id to avoid
  // "FML_DCHECK(states_.find(pointer_data.device) == states_.end());"
  // exception in flutter/engine.
  // This is because "device-id = 0" is used for mouse inputs.
  // See engine/lib/ui/window/pointer_data_packet_converter.cc
  id += 1;

  auto* point = GgeTouchPoint(id);
  if (!point) {
    return;
  }

  // Makes sure we have an existing touch pointer in down state to
  // avoid "FML_DCHECK(iter != states_.end())" exception in flutter/engine.
  // See engine/lib/ui/window/pointer_data_packet_converter.cc
  if (point->event_mask != TouchEvent::kDown &&
      point->event_mask != TouchEvent::kMotion) {
    return;
  }
  point->event_mask = TouchEvent::kUp;

  FlutterPointerEvent event = {
      .struct_size = sizeof(event),
      .phase = FlutterPointerPhase::kUp,
      .timestamp = time * kMicrosecondsPerMillisecond,
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

void FlutterELinuxView::OnTouchMotion(uint32_t time,
                                      int32_t id,
                                      double x,
                                      double y) {
  // Increase device-id to avoid avoid
  // "FML_DCHECK(states_.find(pointer_data.device) == states_.end());"
  // exception in flutter/engine.
  // This is because "device-id = 0" is used for mouse inputs.
  // See engine/lib/ui/window/pointer_data_packet_converter.cc
  id += 1;

  auto trimmed_xy = GetPointerRotation(x, y);
  auto* point = GgeTouchPoint(id);
  if (!point) {
    return;
  }

  // Makes sure we have an existing touch pointer in down state to
  // avoid "FML_DCHECK(iter != states_.end())" exception in flutter/engine.
  // See engine/lib/ui/window/pointer_data_packet_converter.cc
  if (point->event_mask != TouchEvent::kDown &&
      point->event_mask != TouchEvent::kMotion) {
    return;
  }
  point->event_mask = TouchEvent::kMotion;
  point->x = trimmed_xy.first;
  point->y = trimmed_xy.second;

  FlutterPointerEvent event = {
      .struct_size = sizeof(event),
      .phase = FlutterPointerPhase::kMove,
      .timestamp = time * kMicrosecondsPerMillisecond,
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

void FlutterELinuxView::OnTouchCancel() {}

void FlutterELinuxView::OnKeyMap(uint32_t format, int fd, uint32_t size) {
  keyboard_handler_->OnKeymap(format, fd, size);
}

void FlutterELinuxView::OnKey(uint32_t key, bool pressed) {
  keyboard_handler_->OnKey(key, pressed);
  if (pressed) {
    auto code_point = keyboard_handler_->GetCodePoint(key);
    if (!keyboard_handler_->IsTextInputSuppressed(code_point)) {
      textinput_handler_->OnKeyPressed(key, code_point);
    }
  }
}

void FlutterELinuxView::OnKeyModifiers(uint32_t mods_depressed,
                                       uint32_t mods_latched,
                                       uint32_t mods_locked,
                                       uint32_t group) {
  keyboard_handler_->OnModifiers(mods_depressed, mods_latched, mods_locked,
                                 group);
}

void FlutterELinuxView::OnVirtualKey(uint32_t code_point) {
  // Since the keycode cannot be specified, set an invalid value(0).
  constexpr uint32_t kCharKey = 0;
  textinput_handler_->OnKeyPressed(kCharKey, code_point);
}

void FlutterELinuxView::OnVirtualSpecialKey(uint32_t keycode) {
  auto code_point = keyboard_handler_->GetCodePoint(keycode);
  textinput_handler_->OnKeyPressed(keycode, code_point);
}

void FlutterELinuxView::OnScroll(double x,
                                 double y,
                                 double delta_x,
                                 double delta_y,
                                 int scroll_offset_multiplier) {
  auto trimmed_xy = GetPointerRotation(x, y);
  SendScroll(trimmed_xy.first, trimmed_xy.second, delta_x, delta_y,
             scroll_offset_multiplier);
}

void FlutterELinuxView::OnVsync(uint64_t last_frame_time_nanos,
                                uint64_t vsync_interval_time_nanos) {
  engine_->OnVsync(last_frame_time_nanos, vsync_interval_time_nanos);
}

FlutterELinuxView::touch_point* FlutterELinuxView::GgeTouchPoint(int32_t id) {
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
void FlutterELinuxView::SendWindowMetrics(size_t width_px,
                                          size_t height_px,
                                          double dpiScale) const {
  FlutterWindowMetricsEvent event = {};
  event.struct_size = sizeof(event);
  event.width = width_px;
  event.height = height_px;
  event.pixel_ratio = dpiScale;
  engine_->SendWindowMetricsEvent(event);
}

void FlutterELinuxView::SendInitialBounds() {
  PhysicalWindowBounds bounds = binding_handler_->GetPhysicalWindowBounds();
  SendWindowMetrics(bounds.width, bounds.height,
                    binding_handler_->GetDpiScale());
}

// Set's |event_data|'s phase to either kMove or kHover depending on the current
// primary mouse button state.
void FlutterELinuxView::SetEventPhaseFromCursorButtonState(
    FlutterPointerEvent* event_data) const {
  // For details about this logic, see FlutterPointerPhase in the embedder.h
  // file.
  event_data->phase =
      mouse_state_.buttons == 0            ? mouse_state_.flutter_state_is_down
                                                 ? FlutterPointerPhase::kUp
                                                 : FlutterPointerPhase::kHover
      : mouse_state_.flutter_state_is_down ? FlutterPointerPhase::kMove
                                           : FlutterPointerPhase::kDown;
}

void FlutterELinuxView::SendPointerMove(double x_px, double y_px) {
  FlutterPointerEvent event = {};
  event.x = x_px;
  event.y = y_px;
  SetEventPhaseFromCursorButtonState(&event);
  SendPointerEventWithData(event);
}

void FlutterELinuxView::SendPointerDown(double x_px, double y_px) {
  FlutterPointerEvent event = {};
  SetEventPhaseFromCursorButtonState(&event);
  event.x = x_px;
  event.y = y_px;
  SendPointerEventWithData(event);
  SetMouseFlutterStateDown(true);
}

void FlutterELinuxView::SendPointerUp(double x_px, double y_px) {
  FlutterPointerEvent event = {};
  SetEventPhaseFromCursorButtonState(&event);
  event.x = x_px;
  event.y = y_px;
  SendPointerEventWithData(event);
  if (event.phase == FlutterPointerPhase::kUp) {
    SetMouseFlutterStateDown(false);
  }
}

void FlutterELinuxView::SendPointerLeave() {
  FlutterPointerEvent event = {};
  event.phase = FlutterPointerPhase::kRemove;
  SendPointerEventWithData(event);
}

void FlutterELinuxView::SendScroll(double x,
                                   double y,
                                   double delta_x,
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

void FlutterELinuxView::SendPointerEventWithData(
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

void* FlutterELinuxView::ProcResolver(const char* name) {
  return GetRenderSurfaceTarget()->GlProcResolver(name);
}

bool FlutterELinuxView::MakeCurrent() {
  return GetRenderSurfaceTarget()->GLContextMakeCurrent();
}

bool FlutterELinuxView::ClearCurrent() {
  return GetRenderSurfaceTarget()->GLContextClearCurrent();
}

bool FlutterELinuxView::Present() {
  return GetRenderSurfaceTarget()->GLContextPresent(0);
}

bool FlutterELinuxView::PresentWithInfo(const FlutterPresentInfo* info) {
  return GetRenderSurfaceTarget()->GLContextPresentWithInfo(info);
}

void FlutterELinuxView::PopulateExistingDamage(const intptr_t fbo_id,
                                               FlutterDamage* existing_damage) {
  GetRenderSurfaceTarget()->PopulateExistingDamage(fbo_id, existing_damage);
}

uint32_t FlutterELinuxView::GetOnscreenFBO() {
  return GetRenderSurfaceTarget()->GLContextFBO();
}

bool FlutterELinuxView::MakeResourceCurrent() {
  return GetRenderSurfaceTarget()->ResourceContextMakeCurrent();
}

bool FlutterELinuxView::CreateRenderSurface() {
  PhysicalWindowBounds bounds = binding_handler_->GetPhysicalWindowBounds();
  auto impeller_enable = engine_.get()->IsImpellerEnabled();
  return binding_handler_->CreateRenderSurface(bounds.width, bounds.height,
                                               impeller_enable);
}

void FlutterELinuxView::DestroyRenderSurface() {
  binding_handler_->DestroyRenderSurface();
}

ELinuxRenderSurfaceTarget* FlutterELinuxView::GetRenderSurfaceTarget() const {
  return binding_handler_->GetRenderSurfaceTarget();
}

FlutterELinuxEngine* FlutterELinuxView::GetEngine() {
  return engine_.get();
}

int32_t FlutterELinuxView::GetFrameRate() {
  return binding_handler_->GetFrameRate();
}

FlutterTransformation FlutterELinuxView::GetRootSurfaceTransformation() {
  auto degree = binding_handler_->GetRotationDegree();
  if (view_rotation_degree_ != degree) {
    view_rotation_transformation_ = FlutterTransformationMake(degree);
  }
  view_rotation_degree_ = degree;

  auto bounds = binding_handler_->GetPhysicalWindowBounds();
  switch (degree) {
    case 90:
      view_rotation_transformation_.transX = bounds.height;
      break;
    case 180:
      view_rotation_transformation_.transX = bounds.width;
      view_rotation_transformation_.transY = bounds.height;
      break;
    case 270:
      view_rotation_transformation_.transY = bounds.width;
      break;
    default:
      break;
  }
  return view_rotation_transformation_;
}

std::pair<double, double> FlutterELinuxView::GetPointerRotation(double x_px,
                                                                double y_px) {
  auto degree = binding_handler_->GetRotationDegree();
  auto bounds = binding_handler_->GetPhysicalWindowBounds();
  std::pair<double, double> res = {x_px, y_px};

  if (degree == 90) {
    res.first = y_px;
    res.second = bounds.height - x_px;
  } else if (degree == 180) {
    res.first = bounds.width - x_px;
    res.second = bounds.height - y_px;
  } else if (degree == 270) {
    res.first = bounds.width - y_px;
    res.second = x_px;
  }
  return res;
}

void FlutterELinuxView::UpdateHighContrastEnabled(bool enabled) {
  int flags = 0;
  if (enabled) {
    flags |=
        FlutterAccessibilityFeature::kFlutterAccessibilityFeatureHighContrast;
  } else {
    flags &=
        ~FlutterAccessibilityFeature::kFlutterAccessibilityFeatureHighContrast;
  }
  engine_.get()->UpdateAccessibilityFeatures(
      static_cast<FlutterAccessibilityFeature>(flags));
  settings_handler_->UpdateHighContrastMode(enabled);
}

void FlutterELinuxView::UpdateTextScaleFactor(float factor) {
  settings_handler_->UpdateTextScaleFactor(factor);
}

}  // namespace flutter
