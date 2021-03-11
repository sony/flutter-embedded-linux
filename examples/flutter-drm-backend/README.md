# Overview

This is the example of DRM backend.

## Building

```Shell
$ mkdir build
$ cd build
$ cmake -DUSER_PROJECT_PATH=examples/flutter-drm-backend ..
$ make
```

## Running your Flutter app

`FLUTTER_DRM_DEVICE` must be set properly when you use the DRM backend. The default value is `/dev/dri/card0`.

```
$ FLUTTER_DRM_DEVICE="/dev/dri/card1" ./flutter-drm-backend ./sample/build/linux/x64/release/bundle
```
