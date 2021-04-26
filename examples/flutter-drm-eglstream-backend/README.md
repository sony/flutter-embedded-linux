# Overview

This is the example of DRM backend with EGLStream.

## Building

```Shell
$ mkdir build
$ cd build
$ cmake -DUSER_PROJECT_PATH=examples/flutter-drm-eglstream-backend ..
$ cmake --build .
```

## Running Flutter app

You need to switch from GUI which is running X11 or Wayland to the Character User Interface (CUI). In addition, `FLUTTER_DRM_DEVICE` must be set properly. The default value is `/dev/dri/card0`.

```Shell
$ Ctrl + Alt + F3 # Switching to CUI
$ FLUTTER_DRM_DEVICE="/dev/dri/card1" ./flutter-drm-eglstream-backend --bundle=FLUTTER_BUNDLE_PATH
```

Note that replace `FLUTTER_BUNDLE_PATH` with the flutter bundle path you want to use like ./sample/build/linux/x64/release/bundle.

If you want to switch back from CUI to GUI, run `Ctrl + Alt + F2` keys in a terminal.
