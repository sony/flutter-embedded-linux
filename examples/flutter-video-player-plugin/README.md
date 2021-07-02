# Overview
Flutter video player example using video player plugin for Embedded Linux. The interface of the plugin is compatible with [the Flutter official video player plugin](https://github.com/flutter/plugins/tree/master/packages/video_player/video_player). Also, This plugin is temporarily putting here and there are still some bugs. It will be moved to another new repo soon.

The procedure described here is a provisional step until you build a flutter app for Embedded Linux using this embedder with another tool.

![image](https://user-images.githubusercontent.com/62131389/124210378-43f06400-db26-11eb-8723-40dad0eb67b0.png)

## Quick start

### Install libraries
This plugin uses GStreamer internally.

```Shell
$ sudo apt install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer1.0-0 gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav
```

### Building the embedder

```Shell
$ mkdir build
$ cd build
$ cmake -DUSER_PROJECT_PATH=examples/flutter-video-player-plugin ..
$ cmake --build .
```

### Building Flutter app with debug mode

```Shell
$ git clone https://github.com/flutter/plugins.git
$ cd plugins/packages/video_player/video_player
$ flutter build bundle --asset-dir=./bundle/data/flutter_assets
$ cp <path_to_flutter_sdk_install>/bin/cache/artifacts/engine/linux-*/icudtl.dat ./bundle/data
```

### Running Flutter app

```Shell
$ cd <path_to_build_embedder>
$ LD_LIBRARY_PATH=./plugins/video_player/ ./flutter-client -b ./bundle/
```
