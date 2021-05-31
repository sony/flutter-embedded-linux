// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux_embedded/plugins/mouse_cursor_plugin.h"

#include "flutter/shell/platform/common/client_wrapper/include/flutter/standard_method_codec.h"

// Avoids the following build error:
// ----------------------------------------------------------------
//  error: expected unqualified-id
//    result->Success(document);
//            ^
// /usr/include/X11/X.h:350:21: note: expanded from macro 'Success'
// #define Success            0    /* everything's okay */
// ----------------------------------------------------------------
#if defined(DISPLAY_BACKEND_TYPE_X11)
#undef Success
#endif

namespace flutter {

namespace {
constexpr char kChannelName[] = "flutter/mousecursor";

constexpr char kActivateSystemCursorMethod[] = "activateSystemCursor";

constexpr char kKindKey[] = "kind";
}  // namespace

MouseCursorPlugin::MouseCursorPlugin(BinaryMessenger* messenger,
                                     WindowBindingHandler* delegate)
    : channel_(std::make_unique<MethodChannel<EncodableValue>>(
          messenger, kChannelName, &StandardMethodCodec::GetInstance())),
      delegate_(delegate) {
  channel_->SetMethodCallHandler(
      [this](const MethodCall<EncodableValue>& call,
             std::unique_ptr<MethodResult<EncodableValue>> result) {
        HandleMethodCall(call, std::move(result));
      });
}

void MouseCursorPlugin::HandleMethodCall(
    const MethodCall<EncodableValue>& method_call,
    std::unique_ptr<MethodResult<EncodableValue>> result) {
  const std::string& method = method_call.method_name();
  if (method.compare(kActivateSystemCursorMethod) == 0) {
    const auto& arguments = std::get<EncodableMap>(*method_call.arguments());
    auto kind_iter = arguments.find(EncodableValue(std::string(kKindKey)));
    if (kind_iter == arguments.end()) {
      result->Error("Argument error",
                    "Missing argument while trying to activate system cursor");
      return;
    }
    const auto& kind = std::get<std::string>(kind_iter->second);
    delegate_->UpdateFlutterCursor(kind);
    result->Success();
  } else {
    result->NotImplemented();
  }
}

}  // namespace flutter
