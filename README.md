# Flutter embedder for embedded Linux systems
This project was created to develop **non-official** embedded Linux embeddings of [Flutter](https://flutter.dev/). This embedder is focusing on embedded Linux system use cases. It is also implemented based on Flutter desktop for Windows and has some unique features to use it in embedded systems.

## Objective & Goal
Our objective is to use Flutter in embedded systems. We're developing this embedder to use Flutter in embedded products. Ultimately we would like to propose and contribute this software to the mainline of [Flutter Engine](https://github.com/flutter/engine), which means we would like to add embedded systems support into Flutter officially for all embedded developers. Please note that this is just our ideal, not the official opinion of the Flutter community.

We would be grateful if you could give us feedback on bugs and new feature requests. We would like to cover specifications of general-purpose embedded systems.

## Features
- Suitable for use in embedded systems
  - A few dependent libraries
  - Lightweight than Flutter desktop for Linux (Not using X11 and GTK)
  - The main target of this embedder is Arm64 devices. We haven't confirmed in Arm 32bit (ARMv7, armhf) devices
- [Wayland](https://wayland.freedesktop.org/) backend support
- Direct rendering module ([DRM](https://en.wikipedia.org/wiki/Direct_Rendering_Manager)) backend support
  - [x] Generic Buffer Management ([GBM](https://en.wikipedia.org/wiki/Mesa_(computer_graphics)))
  - [ ] [EGLStream](https://docs.nvidia.com/drive/drive_os_5.1.6.1L/nvvib_docs/index.html#page/DRIVE_OS_Linux_SDK_Development_Guide/Graphics/graphics_eglstream_user_guide.html) for NVIDIA devices (coming soon)
- Keyboard, mouse and touch inputs support
- Equivalent quality to Flutter desktops
- API compatibility with Flutter desktop for Windows and GLFW
  - APIs such as MethodChannel and EventChannel are completely the same with them

## Supported platforms
This embedder supports x64 and Arm64 (aarch64, ARMv8) architectures on Linux which supports either Wayland backend or DRM backend.

## Contributing
**Now, we cannot accept your Pull Request (PR).** Because We are building a system (e.g. CLA) to accept PRs, so please wait for a while the system is getting ready! However, we are always welcome to report bugs and request new features by creating issues.

With the assumption, our final goal of this software openly is to be merged this embedder into [Flutter Engine](https://github.com/flutter/engine) after getting feedbacks. And [Google CLA](https://cla.developers.google.com/about/google-corporate) will be required when we do that in the future. Therefore, we cannot easily accept an external PR. However, you can free to create issues for reporting bugs and requesting new features.

See also: [Contributing to the Flutter engine](https://github.com/flutter/engine/blob/master/CONTRIBUTING.md)

# Contents

## 1. Install libraries

#### Mandatory

- clang (for building)
- cmake (for building)
- build-essential (for building)
- pkg-config (for building)
- EGL
- xkbcommon
- OpenGL ES (>=2.0)

```Shell
$ sudo apt install clang build-essential pkg-config libegl-dev libxkbcommon-dev libgles2-mesa-dev
```

#### Only when you use Wayland backend
- libwayland

```Shell
$ sudo apt install libwayland-dev
```

#### Only when you use weston desktop-shell
- weston (>=6.0.1)

```Shell
$ sudo apt install weston
```

#### Only when you use DRM backend
- libdrm
- libgbm
- libinput
- libudev
- libsystemd

```Shell
$ sudo apt install libdrm-dev libgbm-dev libinput-dev libudev-dev libsystemd-dev
```

#### Install Flutter core embedder library

This embedder requres `libflutter_engine.so` (Flutter embedder library). You need to install `libflutter_engine.so` to `/usr/lib`. See: [Building Flutter Engine embedder](./BUILDING-CORE-EMBEDDER.md)

Or you can download a specific Flutter Engine by the following steps, but it's limited to debug mode and x64 targets.

[Step1] Check the version (SHA) of the channel you want to use.
- [master channel](https://raw.githubusercontent.com/flutter/flutter/master/bin/internal/engine.version)
- [dev channel](https://raw.githubusercontent.com/flutter/flutter/dev/bin/internal/engine.version)
- [beta channel](https://raw.githubusercontent.com/flutter/flutter/beta/bin/internal/engine.version)
- [stable channel](https://raw.githubusercontent.com/flutter/flutter/stable/bin/internal/engine.version)

[Step2] Download Flutter Engine embedder library. Note that replace `FLUTTER_ENGINE` with the SHA of the Flutter engine you want to use.
```Shell
$ curl -O https://storage.googleapis.com/flutter_infra/flutter/FLUTTER_ENGINE/linux-x64/linux-x64-embedder
```

## 2. Examples
There are sample projects in [`examples`](./examples) directory. You can also comunicate with Dart code by using the plugin APIs with the same specifications as with Flutter desktops for Windows.

## 3. Building

### Build for Wayland backend (Stand-alone application)

```Shell
$ mkdir build
$ cd build
$ cmake -DUSER_PROJECT_PATH=examples/flutter-wayland-client ..
$ make
```

### Build for DRM backend

```Shell
$ mkdir build
$ cd build
$ cmake -DUSER_PROJECT_PATH=examples/flutter-drm-backend ..
$ make
```

### Build for Wayland backend (weston desktop-shell)

```Shell
$ mkdir build
$ cd build
$ cmake -DUSER_PROJECT_PATH=examples/flutter-weston-desktop-shell ..
$ make
```

This binary will run as a desktop-shell by setting `weston.ini` when Weston starts. See [Settings of weston.ini file](#4-settings-of-westonini-file-only-when-you-use-weston-desktop-shell).

### User configuration parameters (CMAKE options)

Please edit `cmake/user_config.cmake` file.

| Option | Description |
| ------------- | ------------- |
| USE_DRM | Use DRM backend instead of Wayland |
| DESKTOP_SHELL | Work as weston desktop-shell |
| USE_VIRTUAL_KEYBOARD | Use Virtual Keyboard (only when you use `DESKTOP_SHELL`) |
| USE_GLES3 | Use OpenGLES3 instead of OpenGLES2 |

## 4. Running your Flutter app

### Install Flutter SDK

Please note that you must use the same version that you built Flutter embedder for. See also: [Building Flutter Engine embedder](./BUILDING-CORE-EMBEDDER.md)

```Shell
$ git clone https://github.com/flutter/flutter
$ sudo mv flutter /opt/
$ export PATH=$PATH:/opt/flutter/bin
```

### Build your Flutter app

Here introduce how to build the flutter sample app.

#### for x64 targets on x64 hosts / for Arm64 targets on Arm64 hosts

```Shell
$ flutter create sample
$ cd sample/
$ flutter build linux
$ cd ..
```

#### for Arm64 targets on x64 hosts

Comming soon. We are contributing to support this now. See: https://github.com/flutter/flutter/issues/74929

### Run Fluter app

```Shell
$ ./flutter-client ./sample/build/linux/x64/release/bundle
```

#### Note
You need to run this program by a user who has the permission to access the input devices(/dev/input/xxx), if you use the DRM backend. Generally, it is a root user or a user who belongs to an input group.

## 5. Settings of weston.ini file (Only when you use weston desktop-shell)

Sample file can be found [examples/config/weston.ini](./examples/config/weston.ini). See also `man weston.ini`.

### shell section

Sets the following when this embedder works as a desktop-shell on weston.

| Field | Description |
| ------------- | ------------- |
| client | ${path to the binary}/flutter-desktop-shell |

### extended section section

An extended section for this embedder. This section is valid only for desktop-shell. The entries that can appear in this section are:

| Field | Description |
| ------------- | ------------- |
| show-cursor | Set whether to show mouse cursor (boolean) |
| flutter-project-path | Set an absolute path or relative path from the binary file to Flutter project path (string) |
