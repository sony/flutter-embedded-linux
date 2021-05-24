#include "external_texture_test_plugin.h"

#include <flutter/standard_method_codec.h>
#include <unistd.h>

namespace {

static constexpr char kChannelName[] = "external_texture_test";
static constexpr char kMethodNameInitialize[] = "initialize";
static constexpr char kTextureId[] = "textureId";

ColorBarTexture::ColorBarTexture() : request_count_(0) {
  constexpr size_t width = 1024;
  constexpr size_t height = 640;
  pixels_.reset(new uint8_t[width * height * 4]);

  buffer_ = std::make_unique<FlutterDesktopPixelBuffer>();
  buffer_->buffer = pixels_.get();
  buffer_->width = width;
  buffer_->height = height;
}

const FlutterDesktopPixelBuffer *ColorBarTexture::CopyBuffer(size_t width,
                                                             size_t height) {
  PrepareBuffer();
  request_count_++;
  return buffer_.get();
}

void ColorBarTexture::PrepareBuffer() {
  constexpr uint32_t kColorData[] = {0xFFFFFFFF, 0xFF00C0C0, 0xFFC0C000,
                                     0xFF00C000, 0xFFC000C0, 0xFF0000C0,
                                     0xFFC00000, 0xFF000000};
  auto data_num = sizeof(kColorData) / sizeof(uint32_t);

  auto *buffer = buffer_.get();
  auto pixel = const_cast<uint32_t *>(
      reinterpret_cast<const uint32_t *>(buffer->buffer));
  auto width = buffer->width;
  auto height = buffer->height;
  auto column_width = width / data_num;
  auto offset = request_count_ % 8;

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      auto index = (j / column_width) + offset;
      index -= (index < data_num) ? 0 : data_num;
      *(pixel++) = kColorData[index];
    }
  }
}

// static
void ExternalTextureTestPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrar *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), kChannelName,
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<ExternalTextureTestPlugin>(
      registrar->texture_registrar());

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

void ExternalTextureTestPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  const std::string &method_name = method_call.method_name();

  if (method_name.compare(kMethodNameInitialize) == 0) {
    color_bar_texture_ = std::make_unique<ColorBarTexture>();
    texture_ =
        std::make_unique<flutter::TextureVariant>(flutter::PixelBufferTexture(
            [this](size_t width,
                   size_t height) -> const FlutterDesktopPixelBuffer * {
              return color_bar_texture_->CopyBuffer(width, height);
            }));
    int64_t texture_id = textures_->RegisterTexture(texture_.get());

    auto response = flutter::EncodableValue(flutter::EncodableMap{
        {flutter::EncodableValue(kTextureId),
         flutter::EncodableValue(texture_id)},
    });
    result->Success(response);

    for (int i = 0; i < 8; i++) {
      sleep(2);
      textures_->MarkTextureFrameAvailable(texture_id);
    }
  } else {
    result->NotImplemented();
  }
}

}  // namespace

void ExternalTextureTestPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  ExternalTextureTestPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
