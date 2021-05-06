# Embedded Linux embedding for Flutter
[![build-test](https://github.com/sony/flutter-embedded-linux/actions/workflows/build-test.yml/badge.svg)](https://github.com/sony/flutter-embedded-linux/actions/workflows/build-test.yml)

This project was created to develop **non-official** embedded Linux embeddings of [Flutter](https://flutter.dev/). This embedder is focusing on embedded Linux system use cases. It is also implemented based on Flutter desktop for Windows and has some unique features to use it in embedded systems.

## Objective & Goal
Our objective is to use Flutter in embedded systems. We're developing this embedder to use Flutter in embedded products. Ultimately we would like to propose and contribute this software to the mainline of [Flutter Engine](https://github.com/flutter/engine), which means we would like to add embedded systems support into Flutter officially for all embedded developers. Please note that this is just our ideal, not the official opinion of the Flutter community.

We would be grateful if you could give us feedback on bugs and new feature requests. We would like to cover specifications of general-purpose embedded systems.

## Features
- Flutter embedder optimized for Embedded Systems
  - Minimal dependent libraries
  - Lightweight than Flutter desktop for Linux (Not using X11 and GTK)
  - The main target of this embedder is Arm64 devices. We haven't confirmed in Arm 32bit (ARMv7, armhf) devices
- Display backend support
  - [Wayland](https://wayland.freedesktop.org/)
  - Direct rendering module ([DRM](https://en.wikipedia.org/wiki/Direct_Rendering_Manager))
    - Generic Buffer Management ([GBM](https://en.wikipedia.org/wiki/Mesa_(computer_graphics)))
    - [EGLStream](https://docs.nvidia.com/drive/drive_os_5.1.6.1L/nvvib_docs/index.html#page/DRIVE_OS_Linux_SDK_Development_Guide/Graphics/graphics_eglstream_user_guide.html) for NVIDIA devices
  - X11
    - For developing Flutter apps on Linux desktops purposes. It is not intended for use in embedded systems. 
- Always single window fullscreen
  - You can choose always-fullscreen or flexible-screen (any size) only when using Wayland/X11 backend
- Keyboard, mouse and touch inputs support
- Equivalent quality to Flutter desktops
- API compatibility with Flutter desktop for Windows and GLFW
  - APIs such as MethodChannel and EventChannel are completely the same with them

## Supported platforms
This embedder supports x64 and Arm64 (aarch64, ARMv8) architectures on Linux which supports either Wayland backend or DRM backend.

### Tested devices

| Board / SoC | Vendor | OS / BSP | Backend | Status |
| :-------------: | :-------------: | :-------------: | :-------------: | :-------------: |
| Desktop (x86_64) | Intel | Ubuntu18.04 | Wayland | :heavy_check_mark: |
| Desktop (x86_64) | Intel | Ubuntu18.04 | DRM | :heavy_check_mark: |
| Desktop (x86_64) | Intel | Ubuntu18.04 | X11 | :heavy_check_mark: |
| QEMU (x86_64) | QEMU | [AGL (Automotive Grade Linux)](https://wiki.automotivelinux.org/) koi | Wayland | :heavy_check_mark: |
| QEMU (x86_64) | QEMU | [AGL (Automotive Grade Linux)](https://wiki.automotivelinux.org/) koi | DRM | :heavy_check_mark: |
| [Jetson Nano](https://developer.nvidia.com/embedded/jetson-nano-developer-kit) | NVIDIA | JetPack 4.3 | Wayland | :heavy_check_mark: |
| [Jetson Nano](https://developer.nvidia.com/embedded/jetson-nano-developer-kit) | NVIDIA | JetPack 4.3 | DRM | :heavy_check_mark: ([#1](https://github.com/sony/flutter-embedded-linux/issues/1))|
| [Raspberry Pi 4 Model B](https://www.raspberrypi.org/products/raspberry-pi-4-model-b/) | Raspberry Pi Foundation | Ubuntu 20.10 | Wayland | :heavy_check_mark: |
| [Raspberry Pi 4 Model B](https://www.raspberrypi.org/products/raspberry-pi-4-model-b/) | Raspberry Pi Foundation | Ubuntu 20.10 | DRM | :heavy_check_mark: ([#9](https://github.com/sony/flutter-embedded-linux/issues/9)) |
| [i.MX 8MQuad EVK](https://www.nxp.com/design/development-boards/i-mx-evaluation-and-development-boards/evaluation-kit-for-the-i-mx-8m-applications-processor:MCIMX8M-EVK) | NXP | Sumo (kernel 4.14.98) | Wayland | :heavy_check_mark: |
| [i.MX 8M Mini EVKB](https://www.nxp.com/design/development-boards/i-mx-evaluation-and-development-boards/evaluation-kit-for-the-i-mx-8m-mini-applications-processor:8MMINILPD4-EVK) | NXP | Zeus (kernel 5.4.70) | Wayland | :heavy_check_mark: |
| Zynq | Xilinx | - | - | Not tested |
| [RB5 Development Kit](https://developer.qualcomm.com/qualcomm-robotics-rb5-kit) | Qualcomm | - | - | Not tested |

Note
 - i.MX 8M platforms don't support applications using EGL on GBM, which means the DRM-GBM backend won't work on i.MX 8M devices.

### Tested Wayland compositors
|||||||||||
| :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: |
[Weston](https://gitlab.freedesktop.org/wayland/weston/-/blob/master/README.md) | :heavy_check_mark: | [Sway](https://swaywm.org/) | :heavy_check_mark: | [Wayfire](https://wayfire.org/) | :heavy_check_mark: | [Gnome](https://www.gnome.org/) | :heavy_check_mark: | [Phosh](https://source.puri.sm/Librem5/phosh) | :heavy_check_mark: |
| [Cage](https://www.hjdskes.nl/projects/cage/) | :heavy_check_mark: | [Lomiri](https://lomiri.com/) | :heavy_check_mark: | [Plasma Wayland](https://community.kde.org/Plasma/Wayland) | :heavy_check_mark: | [Plasma Mobile](https://www.plasma-mobile.org/) | :heavy_check_mark: | [GlacierUX](https://wiki.merproject.org/wiki/Nemo/Glacier) | :x: |

## Contributing
**Now, we cannot accept any Pull Request (PR).** Because We are building a system (e.g. CLA) to accept PRs, so please wait for a while the system is getting ready! However, we are always welcome to report bugs and request new features by creating issues.

With the assumption, our final goal of this software openly is to be merged this embedder into [Flutter Engine](https://github.com/flutter/engine) after getting feedbacks. And [Google CLA](https://cla.developers.google.com/about/google-corporate) will be required when we do that in the future. Therefore, we cannot easily accept an external PR. However, you can free to create issues for reporting bugs and requesting new features.

See also: [Contributing to the Flutter engine](https://github.com/flutter/engine/blob/master/CONTRIBUTING.md)

## Documentation
Documentation for this embedder is under the [doc](./doc/README.md) directory.
