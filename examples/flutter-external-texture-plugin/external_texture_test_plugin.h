#ifndef EXAMPLES_FLUTTER_EXTERNAL_TEXTURE_PLUGIN_EXTERNAL_TEXTURE_TEST_PLUGIN_H_
#define EXAMPLES_FLUTTER_EXTERNAL_TEXTURE_PLUGIN_EXTERNAL_TEXTURE_TEST_PLUGIN_H_

#include "flutter/shell/platform/common/client_wrapper/include/flutter/method_channel.h"
#include "flutter/shell/platform/common/client_wrapper/include/flutter/plugin_registrar.h"

namespace {
class ColorBarTexture {
 public:
  ColorBarTexture();
  virtual ~ColorBarTexture() {}
  const FlutterDesktopPixelBuffer* CopyBuffer(size_t width, size_t height);

 private:
  void PrepareBuffer();

  std::unique_ptr<FlutterDesktopPixelBuffer> buffer_;
  std::unique_ptr<uint8_t> pixels_;
  int32_t request_count_;
};

class ExternalTextureTestPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar);

  ExternalTextureTestPlugin(flutter::TextureRegistrar* textures)
      : textures_(textures), texture_(nullptr), color_bar_texture_(nullptr) {}

  virtual ~ExternalTextureTestPlugin() {}

 private:
  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  flutter::TextureRegistrar* textures_;
  std::unique_ptr<flutter::TextureVariant> texture_;
  std::unique_ptr<ColorBarTexture> color_bar_texture_;
};

}  // namespace

void ExternalTextureTestPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar);

#endif  // EXAMPLES_FLUTTER_EXTERNAL_TEXTURE_PLUGIN_EXTERNAL_TEXTURE_TEST_PLUGIN_H_
