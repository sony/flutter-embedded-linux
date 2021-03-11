# Overview

This is the example of DRM backend.

## Building

```Shell
$ mkdir build
$ cd build
$ cmake -DUSER_PROJECT_PATH=examples/flutter-drm-backend ..
$ make
```

## Running Flutter app

`FLUTTER_DRM_DEVICE` must be set properly when you use the DRM backend. The default value is `/dev/dri/card0`.

```Shell
$ FLUTTER_DRM_DEVICE="/dev/dri/card1" ./flutter-drm-backend FLUTTER_BUNDLE_PATH
```

Note that replace `FLUTTER_BUNDLE_PATH` with the flutter bundle path you want to use like ./sample/build/linux/x64/release/bundle.
