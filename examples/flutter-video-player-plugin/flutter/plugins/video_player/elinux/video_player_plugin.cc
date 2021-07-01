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
#include <unistd.h>

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

constexpr char kEncodableValuekeyResult[] = "result";

class VideoPlayerPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar);

  VideoPlayerPlugin(flutter::PluginRegistrar* plugin_registrar,
                    flutter::TextureRegistrar* texture_registrar)
      : plugin_registrar_(plugin_registrar),
        texture_registrar_(texture_registrar) {}
  virtual ~VideoPlayerPlugin() {}

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
  void HandleSetMixWithOthersMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);
  void HandleCreateMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);
  void HandleDisposeMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);
  void HandleSetLoopingMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);
  void HandleSetVolumeMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);
  void HandlePauseMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);
  void HandlePlayMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);
  void HandlePositionMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);
  void HandleSetPlaybackSpeedMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);
  void HandleSeekToMethodCall(
      const flutter::EncodableValue& message,
      flutter::MessageReply<flutter::EncodableValue> reply);

  void SendInitializedEventMessage(int64_t texture_id);
  void SendPlayCompletedEventMessage(int64_t texture_id);

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
            registrar->messenger(), kVideoPlayerApiChannelPositionName,
            &flutter::StandardMessageCodec::GetInstance());
    channel->SetMessageHandler(
        [plugin_pointer = plugin.get()](const auto& message, auto reply) {
          plugin_pointer->HandlePositionMethodCall(message, reply);
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

  registrar->AddPlugin(std::move(plugin));
}

void VideoPlayerPlugin::HandleInitializeMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  flutter::EncodableMap value;
  value.emplace(flutter::EncodableValue(kEncodableValuekeyResult),
                flutter::EncodableValue());
  reply(flutter::EncodableValue(value));
}

void VideoPlayerPlugin::HandleSetMixWithOthersMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  // todo: implements here.

  flutter::EncodableMap value;
  value.emplace(flutter::EncodableValue(kEncodableValuekeyResult),
                flutter::EncodableValue());
  reply(flutter::EncodableValue(value));
}

void VideoPlayerPlugin::HandleCreateMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  auto meta = CreateMessage::FromMap(message);
  std::string uri;
  if (meta.GetAsset().empty()) {
    uri = meta.GetUri();
  } else {
    // todo:
  }

  auto player = std::make_unique<FlutterVideoPlayer>();
  player->buffer = std::make_unique<FlutterDesktopPixelBuffer>();
  player->texture =
      std::make_unique<flutter::TextureVariant>(flutter::PixelBufferTexture(
          [player = player.get()](
              size_t width, size_t height) -> const FlutterDesktopPixelBuffer* {
            player->buffer->width = player->player->GetWidth();
            player->buffer->height = player->player->GetHeight();
            player->buffer->buffer = player->player->GetFrameBuffer();
            return player->buffer.get();
          }));
  const auto texture_id =
      texture_registrar_->RegisterTexture(player->texture.get());
  player->texture_id = texture_id;
  {
    auto event_channel =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            plugin_registrar_->messenger(),
            kVideoPlayerVideoEventsChannelName + std::to_string(texture_id),
            &flutter::StandardMethodCodec::GetInstance());
    auto event_channel_handler = std::make_unique<
        flutter::StreamHandlerFunctions<flutter::EncodableValue>>(
        [player = player.get(), host = this](
            const flutter::EncodableValue* arguments,
            std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&&
                events)
            -> std::unique_ptr<
                flutter::StreamHandlerError<flutter::EncodableValue>> {
          player->event_sink = std::move(events);
          host->SendInitializedEventMessage(player->texture_id);
          return nullptr;
        },
        [player = player.get()](const flutter::EncodableValue* arguments)
            -> std::unique_ptr<
                flutter::StreamHandlerError<flutter::EncodableValue>> {
          player->event_sink = nullptr;
          return nullptr;
        });
    event_channel->SetStreamHandler(std::move(event_channel_handler));
    player->event_channel = std::move(event_channel);
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

    if (players_.size() == 0) {
      GstVideoPlayer::GstLibraryLoad();
    }
    player->player =
        std::make_unique<GstVideoPlayer>(uri, std::move(player_handler));
    players_[texture_id] = std::move(player);
  }

  TextureMessage result;
  result.SetTextureId(texture_id);

  flutter::EncodableMap value;
  value.emplace(flutter::EncodableValue(kEncodableValuekeyResult),
                result.ToMap());
  reply(flutter::EncodableValue(value));
}

void VideoPlayerPlugin::HandleDisposeMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  auto parameter = TextureMessage::FromMap(message);
  const auto texture_id = parameter.GetTextureId();
  if (players_.find(texture_id) != players_.end()) {
    auto* player = players_[texture_id].get();
    player->event_sink = nullptr;
    player->event_channel->SetStreamHandler(nullptr);
    player->player = nullptr;
    player->buffer = nullptr;
    player->texture = nullptr;
    players_.erase(texture_id);
    texture_registrar_->UnregisterTexture(texture_id);
    if (players_.size() == 0) {
      GstVideoPlayer::GstLibraryUnload();
    }
  } else {
    std::cerr << "Couldn't find the player with texture id: " << texture_id
              << std::endl;
  }

  flutter::EncodableMap value;
  value.emplace(flutter::EncodableValue(kEncodableValuekeyResult),
                flutter::EncodableValue());
  reply(flutter::EncodableValue(value));
}

void VideoPlayerPlugin::HandleSetLoopingMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  auto parameter = LoopingMessage::FromMap(message);
  const auto texture_id = parameter.GetTextureId();
  if (players_.find(texture_id) != players_.end()) {
    players_[texture_id]->player->SetAutoRepeat(parameter.GetIsLooping());
  } else {
    std::cerr << "Couldn't find the player with texture id: " << texture_id
              << std::endl;
  }

  flutter::EncodableMap result;
  result.emplace(flutter::EncodableValue(kEncodableValuekeyResult),
                 flutter::EncodableValue());
  reply(flutter::EncodableValue(result));
}

void VideoPlayerPlugin::HandleSetVolumeMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  auto parameter = VolumeMessage::FromMap(message);
  const auto texture_id = parameter.GetTextureId();
  if (players_.find(texture_id) != players_.end()) {
    players_[texture_id]->player->SetVolume(parameter.GetVolume());
  } else {
    std::cerr << "Couldn't find the player with texture id: " << texture_id
              << std::endl;
  }

  flutter::EncodableMap result;
  result.emplace(flutter::EncodableValue(kEncodableValuekeyResult),
                 flutter::EncodableValue());
  reply(flutter::EncodableValue(result));
}

void VideoPlayerPlugin::HandlePauseMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  auto parameter = TextureMessage::FromMap(message);
  const auto texture_id = parameter.GetTextureId();
  if (players_.find(texture_id) != players_.end()) {
    players_[texture_id]->player->Pause();
  } else {
    std::cerr << "Couldn't find the player with texture id: " << texture_id
              << std::endl;
  }

  flutter::EncodableMap result;
  result.emplace(flutter::EncodableValue(kEncodableValuekeyResult),
                 flutter::EncodableValue());
  reply(flutter::EncodableValue(result));
}

void VideoPlayerPlugin::HandlePlayMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  auto parameter = TextureMessage::FromMap(message);
  const auto texture_id = parameter.GetTextureId();
  if (players_.find(texture_id) != players_.end()) {
    players_[texture_id]->player->Play();
  } else {
    std::cerr << "Couldn't find the player with texture id: " << texture_id
              << std::endl;
  }

  flutter::EncodableMap result;
  result.emplace(flutter::EncodableValue(kEncodableValuekeyResult),
                 flutter::EncodableValue());
  reply(flutter::EncodableValue(result));
}

void VideoPlayerPlugin::HandlePositionMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  auto parameter = TextureMessage::FromMap(message);
  const auto texture_id = parameter.GetTextureId();
  PositionMessage send_message;
  if (players_.find(texture_id) != players_.end()) {
    send_message.SetTextureId(texture_id);
    send_message.SetPosition(
        players_[texture_id]->player->GetCurrentPosition());
  } else {
    std::cerr << "Couldn't find the player with texture id: " << texture_id
              << std::endl;
  }

  flutter::EncodableMap result;
  result.emplace(flutter::EncodableValue(kEncodableValuekeyResult),
                 send_message.ToMap());
  reply(flutter::EncodableValue(result));
}

void VideoPlayerPlugin::HandleSetPlaybackSpeedMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  auto parameter = PlaybackSpeedMessage::FromMap(message);
  const auto texture_id = parameter.GetTextureId();
  if (players_.find(texture_id) != players_.end()) {
    players_[texture_id]->player->SetPlaybackRate(parameter.GetSpeed());
  } else {
    std::cerr << "Couldn't find the player with texture id: " << texture_id
              << std::endl;
  }

  flutter::EncodableMap result;
  result.emplace(flutter::EncodableValue(kEncodableValuekeyResult),
                 flutter::EncodableValue());
  reply(flutter::EncodableValue(result));
}

void VideoPlayerPlugin::HandleSeekToMethodCall(
    const flutter::EncodableValue& message,
    flutter::MessageReply<flutter::EncodableValue> reply) {
  auto parameter = PositionMessage::FromMap(message);
  const auto texture_id = parameter.GetTextureId();
  if (players_.find(texture_id) != players_.end()) {
    players_[texture_id]->player->SetSeek(parameter.GetPosition());
  } else {
    std::cerr << "Couldn't find the player with texture id: " << texture_id
              << std::endl;
  }

  flutter::EncodableMap result;
  result.emplace(flutter::EncodableValue(kEncodableValuekeyResult),
                 flutter::EncodableValue());
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

}  // namespace

void VideoPlayerPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  VideoPlayerPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
