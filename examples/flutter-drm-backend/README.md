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

You need to switch from GUI which is running X11 or Wayland to the Character User Interface (CUI). In addition, `FLUTTER_DRM_DEVICE` must be set properly. The default value is `/dev/dri/card0`.

```Shell
$ Ctrl + Alt + F3 # Switching to CUI
$ FLUTTER_DRM_DEVICE="/dev/dri/card1" ./flutter-drm-backend FLUTTER_BUNDLE_PATH
```

Note that replace `FLUTTER_BUNDLE_PATH` with the flutter bundle path you want to use like ./sample/build/linux/x64/release/bundle.

You need the following command to switch back from CUI to GUI.
```
$ Ctrl + Alt + F2
```
