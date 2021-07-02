// Copyright 2021 Sony Group Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/video_player/video_player_plugin.h"

#include <flutter/basic_message_channel.h>
#include <flutter/encodable_value.h>
#include <flutter/event_channel.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_message_codec.h>
#include <flutter/standard_method_codec.h>

#include <unordered_map>

#include "gst_video_player.h"
#include "messages/messages.h"
#include "video_player_stream_handler_impl.h"

namespace {
constexpr char kVideoPlayerApiChannelInitializeName[] =
    "dev.flutter.pigeon.VideoPlayerApi.initialize";
constexpr char kVideoPlayerApiChannelSetMixWithOthersName[] =
    "dev.flutter.pigeon.VideoPlayerApi.setMixWithOthers";
constexpr char kVideoPlayerApiChannelCreateName[] =
    "dev.flutter.pigeon.VideoPlayerApi.create";
constexpr char kVideoPlayerApiChannelDisposeName[] =
    "dev.flutter.pigeon.VideoPlayerApi.dispose";
constexpr char kVideoPlayerApiChannelSetLoopingName[] =
    "dev.flutter.pigeon.VideoPlayerApi.setLooping";
constexpr char kVideoPlayerApiChannelSetVolumeName[] =
    "dev.flutter.pigeon.VideoPlayerApi.setVolume";
constexpr char kVideoPlayerApiChannelPauseName[] =
    "dev.flutter.pigeon.VideoPlayerApi.pause";
constexpr char kVideoPlayerApiChannelPlayName[] =
    "dev.flutter.pigeon.VideoPlayerApi.play";
constexpr char kVideoPlayerApiChannelPositionName[] =
    "dev.flutter.pigeon.VideoPlayerApi.position";
constexpr char kVideoPlayerApiChannelSetPlaybackSpeedName[] =
    "dev.flutter.pigeon.VideoPlayerApi.setPlaybackSpeed";
constexpr char kVideoPlayerApiChannelSeekToName[] =
    "dev.flutter.pigeon.VideoPlayerApi.seekTo";

constexpr char kVideoPlayerVideoEventsChannelName[] =
    "flutter.io/videoPlayer/videoEvents";

constexpr char kEncodableMapkeyResult[] = "result";
constexpr char kEncodableMapkeyError[] = "error";

class VideoPlayerPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar);

  VideoPlayerPlugin(flutter::PluginRegistrar* plugin_registrar,
                    flutter::TextureRegistrar* texture_registrar)
      : plugin_registrar_(plugin_registrar),
        texture_registrar_(texture_registrar) {
    // Needs to call 'gst_init' that initializing the GStreamer library before
    // using it.
    GstVideoPlayer::GstLibraryLoad();
  }
  virtual ~VideoPlayerPlugin() {
    for (auto itr = players_.begin(); itr != players_.end(); itr++) {
      auto texture_id = itr->first;
      auto* player = itr->second.get();
      player->event_sink = nullptr;
      if (player->event_channel) {
        player->event_channel->SetStreamHandler(nullptr);
      }
      player->player = nullptr;
      player->buffer = nullptr;
      player->texture = nullptr;
      texture_registrar_->UnregisterTexture(texture_id);
    }
    players_.clear();

    GstVideoPlayer::GstLibraryUnload();
  }

 private:
  struct FlutterVideoPlayer {
    int64_t texture_id;
    std::unique_ptr<GstVideoPlayer> player;
    std::unique_ptr<flutter::TextureVariant> texture;
    std::unique_ptr<FlutterDesktopPixelBuffer> buffer;
    std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>>
        event_channel;
    std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> event_sink;
  };

  void HandleInitializeMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);
  void HandleCreateMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);
  void HandleDisposeMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);
  void HandlePauseMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);
  void HandlePlayMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);
  void HandleSetLoopingMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);
  void HandleSetVolumeMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);
  void HandleSetMixWithOthersMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);
  void HandleSetPlaybackSpeedMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);
  void HandleSeekToMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);
  void HandlePositionMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);

  void SendInitializedEventMessage(int64_t texture_id);
  void SendPlayCompletedEventMessage(int64_t texture_id);

  flutter::EncodableValue WrapError(const std::string& message,
                                    const std::string& code = std::string(),
                                    const std::string& details = std::string());

  flutter::PluginRegistrar* plugin_registrar_;
  flutter::TextureRegistrar* texture_registrar_;
  std::unordered_map<int64_t, std::unique_ptr<FlutterVideoPlayer>> players_;
};

// static
void VideoPlayerPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrar* registrar) {
  auto plugin = std::make_unique<VideoPlayerPlugin>(
      registrar, registrar->texture_registrar());

  {
    auto channel =
        std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
            registrar->messenger(), kVideoPlayerApiChannelInitializeName,
            &flutter::StandardMessageCodec::GetInstance());
    channel->SetMessageHandler(
        [plugin_pointer = plugin.get()](const auto& message, auto reply) {
          plugin_pointer->HandleInitializeMethodCall(message, reply);
        });
  }

  {
    auto channel =
        std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
            registrar->messenger(), kVideoPlayerApiChannelCreateName,
            &flutter::StandardMessageCodec::GetInstance());
    channel->SetMessageHandler(
        [plugin_pointer = plugin.get()](const auto& message, auto reply) {
          plugin_pointer->HandleCreateMethodCall(message, reply);
        });
  }

  {
    auto channel =
        std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
            registrar->messenger(), kVideoPlayerApiChannelDisposeName,
            &flutter::StandardMessageCodec::GetInstance());
    channel->SetMessageHandler(
        [plugin_pointer = plugin.get()](const auto& message, auto reply) {
          plugin_pointer->HandleDisposeMethodCall(message, reply);
        });
  }

  {
    auto channel =
        std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
            registrar->messenger(), kVideoPlayerApiChannelPauseName,
            &flutter::StandardMessageCodec::GetInstance());
    channel->SetMessageHandler(
        [plugin_pointer = plugin.get()](const auto& message, auto reply) {
          plugin_pointer->HandlePauseMethodCall(message, reply);
        });
  }

  {
    auto channel =
        std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
            registrar->messenger(), kVideoPlayerApiChannelPlayName,
            &flutter::StandardMessageCodec::GetInstance());
    channel->SetMessageHandler(
        [plugin_pointer = plugin.get()](const auto& message, auto reply) {
          plugin_pointer->HandlePlayMethodCall(message, reply);
        });
  }

  {
    auto channel =
        std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
            registrar->messenger(), kVideoPlayerApiChannelSetLoopingName,
            &flutter::StandardMessageCodec::GetInstance());
    channel->SetMessageHandler(
        [plugin_pointer = plugin.get()](const auto& message, auto reply) {
          plugin_pointer->HandleSetLoopingMethodCall(message, reply);
        });
  }

  {
    auto channel =
        std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
            registrar->messenger(), kVideoPlayerApiChannelSetVolumeName,
            &flutter::StandardMessageCodec::GetInstance());
    channel->SetMessageHandler(
        [plugin_pointer = plugin.get()](const auto& message, auto reply) {
          plugin_pointer->HandleSetVolumeMethodCall(message, reply);
        });
  }

  {
    auto channel =
        std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
            registrar->messenger(), kVideoPlayerApiChannelSetMixWithOthersName,
            &flutter::StandardMessageCodec::GetInstance());
    channel->SetMessageHandler(
        [plugin_pointer = plugin.get()](const auto& message, auto reply) {
          plugin_pointer->HandleSetMixWithOthersMethodCall(message, reply);
        });
  }

  {
    auto channel =
        std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
            registrar->messenger(), kVideoPlayerApiChannelSetPlaybackSpeedName,
            &flutter::StandardMessageCodec::GetInstance());
    channel->SetMessageHandler(
        [plugin_pointer = plugin.get()](const auto& message, auto reply) {
          plugin_pointer->HandleSetPlaybackSpeedMethodCall(message, reply);
        });
  }

  {
    auto channel =
        std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
            registrar->messenger(), kVideoPlayerApiChannelSeekToName,
            &flutter::StandardMessageCodec::GetInstance());
    channel->SetMessageHandler(
        [plugin_pointer = plugin.get()](const auto& message, auto reply) {
          plugin_pointer->HandleSeekToMethodCall(message, reply);
        });
  }

  {
    auto channel =
        std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
            registrar->messenger(), kVideoPlayerApiChannelPositionName,
            &flutter::StandardMessageCodec::GetInstance());
    channel->SetMessageHandler(
        [plugin_pointer = plugin.get()](const auto& message, auto reply) {
          plugin_pointer->HandlePositionMethodCall(message, reply);
        });
  }

  registrar->AddPlugin(std::move(plugin));
}

void VideoPlayerPlugin::HandleInitializeMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  flutter::EncodableMap result;

  result.emplace(flutter::EncodableValue(kEncodableMapkeyResult),
                 flutter::EncodableValue());
  reply(flutter::EncodableValue(result));
}

void VideoPlayerPlugin::HandleCreateMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  auto meta = CreateMessage::FromMap(message);
  std::string uri;
  if (!meta.GetAsset().empty()) {
    // todo: gets propery path of the Flutter project.
    std::string flutter_project_path = "./bundle/data/";
    uri = flutter_project_path + "flutter_assets/" + meta.GetAsset();
  } else {
    uri = meta.GetUri();
  }

  auto instance = std::make_unique<FlutterVideoPlayer>();
  instance->buffer = std::make_unique<FlutterDesktopPixelBuffer>();
  instance->texture =
      std::make_unique<flutter::TextureVariant>(flutter::PixelBufferTexture(
          [instance = instance.get()](
              size_t width, size_t height) -> const FlutterDesktopPixelBuffer* {
            instance->buffer->width = instance->player->GetWidth();
            instance->buffer->height = instance->player->GetHeight();
            instance->buffer->buffer = instance->player->GetFrameBuffer();
            return instance->buffer.get();
          }));
  const auto texture_id =
      texture_registrar_->RegisterTexture(instance->texture.get());
  instance->texture_id = texture_id;
  {
    auto event_channel =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            plugin_registrar_->messenger(),
            kVideoPlayerVideoEventsChannelName + std::to_string(texture_id),
            &flutter::StandardMethodCodec::GetInstance());
    auto event_channel_handler = std::make_unique<
        flutter::StreamHandlerFunctions<flutter::EncodableValue>>(
        [instance = instance.get(), host = this](
            const flutter::EncodableValue* arguments,
            std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&&
                events)
            -> std::unique_ptr<
                flutter::StreamHandlerError<flutter::EncodableValue>> {
          instance->event_sink = std::move(events);
          host->SendInitializedEventMessage(instance->texture_id);
          return nullptr;
        },
        [instance = instance.get()](const flutter::EncodableValue* arguments)
            -> std::unique_ptr<
                flutter::StreamHandlerError<flutter::EncodableValue>> {
          instance->event_sink = nullptr;
          return nullptr;
        });
    event_channel->SetStreamHandler(std::move(event_channel_handler));
    instance->event_channel = std::move(event_channel);
  }
  {
    auto player_handler = std::make_unique<VideoPlayerStreamHandlerImpl>(
        // OnNotifyInitialized
        [texture_id, host = this]() {
          host->SendInitializedEventMessage(texture_id);
        },
        // OnNotifyFrameDecoded
        [texture_id, host = this]() {
          host->texture_registrar_->MarkTextureFrameAvailable(texture_id);
        },
        // OnNotifyCompleted
        [texture_id, host = this]() {
          host->SendPlayCompletedEventMessage(texture_id);
        });
    instance->player =
        std::make_unique<GstVideoPlayer>(uri, std::move(player_handler));
    players_[texture_id] = std::move(instance);
  }

  flutter::EncodableMap value;
  TextureMessage result;
  result.SetTextureId(texture_id);
  value.emplace(flutter::EncodableValue(kEncodableMapkeyResult),
                result.ToMap());
  reply(flutter::EncodableValue(value));
}

void VideoPlayerPlugin::HandleDisposeMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  auto parameter = TextureMessage::FromMap(message);
  const auto texture_id = parameter.GetTextureId();
  flutter::EncodableMap result;

  if (players_.find(texture_id) != players_.end()) {
    auto* player = players_[texture_id].get();
    player->event_sink = nullptr;
    player->event_channel->SetStreamHandler(nullptr);
    player->player = nullptr;
    player->buffer = nullptr;
    player->texture = nullptr;
    players_.erase(texture_id);
    texture_registrar_->UnregisterTexture(texture_id);

    result.emplace(flutter::EncodableValue(kEncodableMapkeyResult),
                   flutter::EncodableValue());
  } else {
    auto error_message = "Couldn't find the player with texture id: " +
                         std::to_string(texture_id);
    result.emplace(flutter::EncodableValue(kEncodableMapkeyError),
                   flutter::EncodableValue(WrapError(error_message)));
  }
  reply(flutter::EncodableValue(result));
}

void VideoPlayerPlugin::HandlePauseMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  auto parameter = TextureMessage::FromMap(message);
  const auto texture_id = parameter.GetTextureId();
  flutter::EncodableMap result;

  if (players_.find(texture_id) != players_.end()) {
    players_[texture_id]->player->Pause();
    result.emplace(flutter::EncodableValue(kEncodableMapkeyResult),
                   flutter::EncodableValue());
  } else {
    auto error_message = "Couldn't find the player with texture id: " +
                         std::to_string(texture_id);
    result.emplace(flutter::EncodableValue(kEncodableMapkeyError),
                   flutter::EncodableValue(WrapError(error_message)));
  }
  reply(flutter::EncodableValue(result));
}

void VideoPlayerPlugin::HandlePlayMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  auto parameter = TextureMessage::FromMap(message);
  const auto texture_id = parameter.GetTextureId();
  flutter::EncodableMap result;

  if (players_.find(texture_id) != players_.end()) {
    players_[texture_id]->player->Play();
    result.emplace(flutter::EncodableValue(kEncodableMapkeyResult),
                   flutter::EncodableValue());
  } else {
    auto error_message = "Couldn't find the player with texture id: " +
                         std::to_string(texture_id);
    result.emplace(flutter::EncodableValue(kEncodableMapkeyError),
                   flutter::EncodableValue(WrapError(error_message)));
  }
  reply(flutter::EncodableValue(result));
}

void VideoPlayerPlugin::HandleSetLoopingMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  auto parameter = LoopingMessage::FromMap(message);
  const auto texture_id = parameter.GetTextureId();
  flutter::EncodableMap result;

  if (players_.find(texture_id) != players_.end()) {
    players_[texture_id]->player->SetAutoRepeat(parameter.GetIsLooping());
    result.emplace(flutter::EncodableValue(kEncodableMapkeyResult),
                   flutter::EncodableValue());
  } else {
    auto error_message = "Couldn't find the player with texture id: " +
                         std::to_string(texture_id);
    result.emplace(flutter::EncodableValue(kEncodableMapkeyError),
                   flutter::EncodableValue(WrapError(error_message)));
  }
  reply(flutter::EncodableValue(result));
}

void VideoPlayerPlugin::HandleSetVolumeMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  auto parameter = VolumeMessage::FromMap(message);
  const auto texture_id = parameter.GetTextureId();
  flutter::EncodableMap result;

  if (players_.find(texture_id) != players_.end()) {
    players_[texture_id]->player->SetVolume(parameter.GetVolume());
    result.emplace(flutter::EncodableValue(kEncodableMapkeyResult),
                   flutter::EncodableValue());
  } else {
    auto error_message = "Couldn't find the player with texture id: " +
                         std::to_string(texture_id);
    result.emplace(flutter::EncodableValue(kEncodableMapkeyError),
                   flutter::EncodableValue(WrapError(error_message)));
  }
  reply(flutter::EncodableValue(result));
}

void VideoPlayerPlugin::HandleSetMixWithOthersMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  // todo: implements here.

  flutter::EncodableMap result;
  result.emplace(flutter::EncodableValue(kEncodableMapkeyResult),
                 flutter::EncodableValue());
  reply(flutter::EncodableValue(result));
}

void VideoPlayerPlugin::HandlePositionMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  auto parameter = TextureMessage::FromMap(message);
  const auto texture_id = parameter.GetTextureId();
  flutter::EncodableMap result;

  if (players_.find(texture_id) != players_.end()) {
    PositionMessage send_message;
    send_message.SetTextureId(texture_id);
    send_message.SetPosition(
        players_[texture_id]->player->GetCurrentPosition());
    result.emplace(flutter::EncodableValue(kEncodableMapkeyResult),
                   send_message.ToMap());
  } else {
    auto error_message = "Couldn't find the player with texture id: " +
                         std::to_string(texture_id);
    result.emplace(flutter::EncodableValue(kEncodableMapkeyError),
                   flutter::EncodableValue(WrapError(error_message)));
  }
  reply(flutter::EncodableValue(result));
}

void VideoPlayerPlugin::HandleSetPlaybackSpeedMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  auto parameter = PlaybackSpeedMessage::FromMap(message);
  const auto texture_id = parameter.GetTextureId();
  flutter::EncodableMap result;

  if (players_.find(texture_id) != players_.end()) {
    players_[texture_id]->player->SetPlaybackRate(parameter.GetSpeed());
    result.emplace(flutter::EncodableValue(kEncodableMapkeyResult),
                   flutter::EncodableValue());
  } else {
    auto error_message = "Couldn't find the player with texture id: " +
                         std::to_string(texture_id);
    result.emplace(flutter::EncodableValue(kEncodableMapkeyError),
                   flutter::EncodableValue(WrapError(error_message)));
  }
  reply(flutter::EncodableValue(result));
}

void VideoPlayerPlugin::HandleSeekToMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  auto parameter = PositionMessage::FromMap(message);
  const auto texture_id = parameter.GetTextureId();
  flutter::EncodableMap result;

  if (players_.find(texture_id) != players_.end()) {
    players_[texture_id]->player->SetSeek(parameter.GetPosition());
    result.emplace(flutter::EncodableValue(kEncodableMapkeyResult),
                   flutter::EncodableValue());
  } else {
    auto error_message = "Couldn't find the player with texture id: " +
                         std::to_string(texture_id);
    result.emplace(flutter::EncodableValue(kEncodableMapkeyError),
                   flutter::EncodableValue(WrapError(error_message)));
  }
  reply(flutter::EncodableValue(result));
}

void VideoPlayerPlugin::SendInitializedEventMessage(int64_t texture_id) {
  if (players_.find(texture_id) == players_.end() ||
      !players_[texture_id]->event_sink) {
    return;
  }

  auto duration = players_[texture_id]->player->GetDuration();
  auto width = players_[texture_id]->player->GetWidth();
  auto height = players_[texture_id]->player->GetHeight();
  flutter::EncodableMap encodables = {
      {flutter::EncodableValue("event"),
       flutter::EncodableValue("initialized")},
      {flutter::EncodableValue("duration"), flutter::EncodableValue(duration)},
      {flutter::EncodableValue("width"), flutter::EncodableValue(width)},
      {flutter::EncodableValue("height"), flutter::EncodableValue(height)}};
  flutter::EncodableValue event(encodables);
  players_[texture_id]->event_sink->Success(event);
}

void VideoPlayerPlugin::SendPlayCompletedEventMessage(int64_t texture_id) {
  if (players_.find(texture_id) == players_.end() ||
      !players_[texture_id]->event_sink) {
    return;
  }

  flutter::EncodableMap encodables = {
      {flutter::EncodableValue("event"), flutter::EncodableValue("completed")}};
  flutter::EncodableValue event(encodables);
  players_[texture_id]->event_sink->Success(event);
}

flutter::EncodableValue VideoPlayerPlugin::WrapError(
    const std::string& message, const std::string& code,
    const std::string& details) {
  flutter::EncodableMap map = {
      {flutter::EncodableValue("message"), flutter::EncodableValue(message)},
      {flutter::EncodableValue("code"), flutter::EncodableValue(code)},
      {flutter::EncodableValue("details"), flutter::EncodableValue(details)}};
  return flutter::EncodableValue(map);
}

}  // namespace

void VideoPlayerPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  VideoPlayerPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
