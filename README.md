# Embedded Linux (eLinux) embedding for Flutter
![image](https://github.com/sony/flutter-elinux/blob/main/doc/images/overview.png)

[![build-test](https://github.com/sony/flutter-embedded-linux/actions/workflows/build-test.yml/badge.svg)](https://github.com/sony/flutter-embedded-linux/actions/workflows/build-test.yml)

This project was created to develop **non-official** embedded Linux embeddings of [Flutter](https://flutter.dev/). This embedder is focusing on embedded Linux (eLinux) system use cases. It is also implemented based on Flutter desktop for Windows and has some unique features to use it in embedded systems.

If you develop flutter apps for eLinux, use [flutter-elinux](https://github.com/sony/flutter-elinux), which is a non-official extension to the [Flutter SDK](https://github.com/flutter/flutter) to build and debug Flutter apps for embedded Linux devices.

### Repositories

- [flutter-elinux](https://github.com/sony/flutter-elinux): Flutter tools for eLinux
- [flutter-elinux-plugins](https://github.com/sony/flutter-elinux-plugins): Flutter plugins for eLinux
- [flutter-embedded-linux](https://github.com/sony/flutter-embedded-linux): eLinux embedding for Flutter
- [meta-flutter](https://github.com/sony/meta-flutter): Yocto recipes of eLinux embedding for Flutter

## Objective & Goal
Our objective is to use Flutter in embedded systems. We're developing this embedder to use Flutter in embedded products. Ultimately we would like to propose and contribute this software to the mainline of [Flutter Engine](https://github.com/flutter/engine), which means we would like to add an embedded systems version into the Flutter repo for all embedded developers. Please note that this is just our ideal, not the official opinion of the Flutter community.

We would be grateful if you could give us feedback on bugs and new feature requests. We would like to cover the specifications of general-purpose embedded systems.

## Features
- Flutter embedder optimized for Embedded Systems
  - Lightweight than Flutter desktop for Linux (Not using X11 and GTK)
  - Minimal dependent libraries
  - The main target of this embedder is Arm64 devices. We haven't confirmed in Arm 32bit (ARMv7, armhf) devices
- Display backend support
  - [Wayland](https://wayland.freedesktop.org/)
  - Direct rendering module ([DRM](https://en.wikipedia.org/wiki/Direct_Rendering_Manager))
    - Generic Buffer Management ([GBM](https://en.wikipedia.org/wiki/Mesa_(computer_graphics)))
    - [EGLStream](https://docs.nvidia.com/drive/drive_os_5.1.6.1L/nvvib_docs/index.html#page/DRIVE_OS_Linux_SDK_Development_Guide/Graphics/graphics_eglstream_user_guide.html) for NVIDIA devices
- Always single window fullscreen
  - You can choose always-fullscreen or flexible-screen (any size) only when using Wayland/X11 backend
- Keyboard, mouse and touch inputs support
- Equivalent quality to Flutter desktops
- API compatibility with Flutter desktop for Windows and GLFW
  - APIs such as MethodChannel and EventChannel are completely the same with them

## Documentation
Documentation for this software can be found at [Wiki](https://github.com/sony/flutter-embedded-linux/wiki).

## Supported platforms
This embedder supports x64 and Arm64 (aarch64, ARMv8) architectures on Linux which supports either Wayland backend or DRM backend. See [Support status](https://github.com/sony/flutter-elinux/wiki/Support-status) for details.
