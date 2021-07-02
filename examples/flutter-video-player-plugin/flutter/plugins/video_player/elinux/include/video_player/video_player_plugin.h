#ifndef PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_INCLUDE_VIDEO_PLAYER_VIDEO_PLAYER_PLUGIN_H_
#define PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_INCLUDE_VIDEO_PLAYER_VIDEO_PLAYER_PLUGIN_H_

#include <flutter_plugin_registrar.h>

#ifdef FLUTTER_PLUGIN_IMPL
#define FLUTTER_PLUGIN_EXPORT __attribute__((visibility("default")))
#else
#define FLUTTER_PLUGIN_EXPORT
#endif

#if defined(__cplusplus)
extern "C" {
#endif

FLUTTER_PLUGIN_EXPORT void VideoPlayerPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar);

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // PACKAGES_VIDEO_PLAYER_VIDEO_PLAYER_ELINUX_INCLUDE_VIDEO_PLAYER_VIDEO_PLAYER_PLUGIN_H_
