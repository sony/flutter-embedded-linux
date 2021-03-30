# Embedded Linux embedding for Flutter
[![build-test](https://github.com/sony/flutter-embedded-linux/actions/workflows/build-test.yml/badge.svg)](https://github.com/sony/flutter-embedded-linux/actions/workflows/build-test.yml)

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
- X11 backend supoort
  - This is for the purpose of developing Flutter apps in Linux desktops. It is not intended for use in embedded systems.
- Always single window fullscreen
  - You can choose always-fullscreen or flexible-screen (any size) only when you use Wayland/X11 backend
- Keyboard, mouse and touch inputs support
- Equivalent quality to Flutter desktops
- API compatibility with Flutter desktop for Windows and GLFW
  - APIs such as MethodChannel and EventChannel are completely the same with them

## Supported platforms
This embedder supports x64 and Arm64 (aarch64, ARMv8) architectures on Linux which supports either Wayland backend or DRM backend.

### Tested devices

| Board/SoC | Vendor | OS/BSP | Backend | Status |
| :-------------: | :-------------: | :-------------: | :-------------: | :-------------: |
| Desktop (x86_64) | Intel | Ubuntu18.04 | Wayland | :heavy_check_mark: |
| Desktop (x86_64) | Intel | Ubuntu18.04 | DRM | :heavy_check_mark: |
| Desktop (x86_64) | Intel | Ubuntu18.04 | X11 | :heavy_check_mark: |
| QEMU (x86_64) | QEMU | [AGL (Automotive Grade Linux)](https://wiki.automotivelinux.org/) jellyfish / koi | Wayland | :heavy_check_mark: |
| QEMU (x86_64) | QEMU | [AGL (Automotive Grade Linux)](https://wiki.automotivelinux.org/) jellyfish / koi | DRM | :heavy_check_mark: |
| [Jetson Nano](https://developer.nvidia.com/embedded/jetson-nano-developer-kit) | NVIDIA | JetPack 4.3 | Wayland | :heavy_check_mark: |
| [Jetson Nano](https://developer.nvidia.com/embedded/jetson-nano-developer-kit) | NVIDIA | JetPack 4.3 | DRM | See: [#1](https://github.com/sony/flutter-embedded-linux/issues/1) |
| [Raspberry Pi 4 Model B](https://www.raspberrypi.org/products/raspberry-pi-4-model-b/) | Raspberry Pi Foundation | Ubuntu 20.10 | Wayland | :heavy_check_mark: |
| [Raspberry Pi 4 Model B](https://www.raspberrypi.org/products/raspberry-pi-4-model-b/) | Raspberry Pi Foundation | Ubuntu 20.10 | DRM | :heavy_check_mark: ([#9](https://github.com/sony/flutter-embedded-linux/issues/9)) |
| [i.MX 8MQuad EVK](https://www.nxp.com/design/development-boards/i-mx-evaluation-and-development-boards/evaluation-kit-for-the-i-mx-8m-applications-processor:MCIMX8M-EVK) | NXP | Sumo (kernel 4.14.98) | Wayland | :heavy_check_mark: |
| [i.MX 8MQuad EVK](https://www.nxp.com/design/development-boards/i-mx-evaluation-and-development-boards/evaluation-kit-for-the-i-mx-8m-applications-processor:MCIMX8M-EVK) | NXP | Sumo (kernel 4.14.98) | DRM | Not tested |
| [i.MX 8M Mini EVKB](https://www.nxp.com/design/development-boards/i-mx-evaluation-and-development-boards/evaluation-kit-for-the-i-mx-8m-mini-applications-processor:8MMINILPD4-EVK) | NXP | Zeus (kernel 5.4.70) | Wayland | :heavy_check_mark: |
| [i.MX 8M Mini EVKB](https://www.nxp.com/design/development-boards/i-mx-evaluation-and-development-boards/evaluation-kit-for-the-i-mx-8m-mini-applications-processor:8MMINILPD4-EVK) | NXP | Zeus (kernel 5.4.70) | DRM | Not tested |
| Zynq | Xilinx | - | - | Not tested |
| [RB5 Development Kit](https://developer.qualcomm.com/qualcomm-robotics-rb5-kit) | Qualcomm | - | - | Not tested |

### Tested Wayland compositors
| Compositor      | Status          |
| :-------------: | :-------------: |
| [Cage](https://www.hjdskes.nl/projects/cage/) | :o: |
| [GlacierUX](https://wiki.merproject.org/wiki/Nemo/Glacier) | :x: |
| [Gnome](https://www.gnome.org/) | :o: |
| [Lomiri](https://lomiri.com/) | :o: |
| [Phosh](https://source.puri.sm/Librem5/phosh) | :o: |
| [Plasma Wayland](https://community.kde.org/Plasma/Wayland) | :o: |
| [Plasma Mobile](https://www.plasma-mobile.org/) | :o: |
| [Sway](https://swaywm.org/) | :o: |
| [Wayfire](https://wayfire.org/) | :o: |
| [Weston](https://gitlab.freedesktop.org/wayland/weston/-/blob/master/README.md) | :o: |

## Contributing
**Now, we cannot accept any Pull Request (PR).** Because We are building a system (e.g. CLA) to accept PRs, so please wait for a while the system is getting ready! However, we are always welcome to report bugs and request new features by creating issues.

With the assumption, our final goal of this software openly is to be merged this embedder into [Flutter Engine](https://github.com/flutter/engine) after getting feedbacks. And [Google CLA](https://cla.developers.google.com/about/google-corporate) will be required when we do that in the future. Therefore, we cannot easily accept an external PR. However, you can free to create issues for reporting bugs and requesting new features.

See also: [Contributing to the Flutter engine](https://github.com/flutter/engine/blob/master/CONTRIBUTING.md)

# Contents

## 1. Install libraries

You need to install the following dependent libraries to build and run. Here introduce how to install the libraries on Debian-based systems like Ubuntu.

#### Mandatory

- clang (for building)
- cmake (for building)
- build-essential (for building)
- pkg-config (for building)
- EGL
- xkbcommon
- OpenGL ES (>=2.0)

```Shell
$ sudo apt install clang cmake build-essential pkg-config libegl1-mesa-dev libxkbcommon-dev libgles2-mesa-dev
```

#### Only when you use Wayland backend
- libwayland
- wayland-protocols (for generating xdg-shell source files)

```Shell
$ sudo apt install libwayland-dev wayland-protocols
```

#### Only when you use weston desktop-shell
- weston (>=6.0.1, see: [#3](https://github.com/sony/flutter-embedded-linux/issues/3))

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

#### Only when you use x11 backend
- x11-xcb
- xcb

```Shell
$ sudo apt install libx11-xcb-dev
```

#### Install Flutter Engine library

This embedder requres `libflutter_engine.so` (Flutter embedder library). You need to install `libflutter_engine.so` in `/usr/lib` to build. See: [Building Flutter Engine embedder](./BUILDING-ENGINE-EMBEDDER.md)

Or you can download a specific pre-built Flutter Engine from Google's infra by the following steps, but it's limited to **debug mode** and **x64** targets.

Step 1) Check the version (SHA) of the channel you want to use.
- [master channel](https://raw.githubusercontent.com/flutter/flutter/master/bin/internal/engine.version)
- [dev channel](https://raw.githubusercontent.com/flutter/flutter/dev/bin/internal/engine.version)
- [beta channel](https://raw.githubusercontent.com/flutter/flutter/beta/bin/internal/engine.version)
- [stable channel](https://raw.githubusercontent.com/flutter/flutter/stable/bin/internal/engine.version)

Step 2) Download Flutter Engine embedder library. Note that replace `FLUTTER_ENGINE` with the SHA of the Flutter engine you want to use.
```Shell
$ curl -O https://storage.googleapis.com/flutter_infra/flutter/FLUTTER_ENGINE/linux-x64/linux-x64-embedder
```

Step 3) Install the library. Note that the downloaded library is only **debug mode** and for **x64** targets. 
```Shell
$ unzip ./linux-x64-embedder
$ sudo cp ./libflutter_engine.so /usr/lib
```

## 2. Examples
There are sample projects in [`examples`](./examples) directory. You can also comunicate with Dart code by using the plugin APIs with the same specifications as with Flutter desktops for Windows.

## 3. Building

### Build for Wayland backend (Stand-alone Wayland app)

```Shell
$ mkdir build
$ cd build
$ cmake -DUSER_PROJECT_PATH=examples/flutter-wayland-client ..
$ cmake --build .
```

### Build for DRM backend

```Shell
$ mkdir build
$ cd build
$ cmake -DUSER_PROJECT_PATH=examples/flutter-drm-backend ..
$ cmake --build .
```

### Build for x11 backend

Basically, the x11 backend is just only for debugging and developing Flutter apps on desktops. And it's still being implemented now.

```Shell
$ mkdir build
$ cd build
$ cmake -DUSER_PROJECT_PATH=examples/flutter-x11-client ..
$ cmake --build .
```

### Build for Wayland backend (Weston desktop-shell)

This binary will run as a desktop-shell by setting `weston.ini` when Weston starts. See [Settings of weston.ini file](#5-settings-of-westonini-file-only-when-you-use-weston-desktop-shell).

```Shell
$ mkdir build
$ cd build
$ cmake -DUSER_PROJECT_PATH=examples/flutter-weston-desktop-shell ..
$ cmake --build .
```

### User configuration parameters (CMAKE options)

Please edit `cmake/user_config.cmake` file.

| Option | Description |
| ------------- | ------------- |
| USE_DRM | Use DRM backend instead of Wayland |
| USE_X11 | Use X11 backend instead of Wayland |
| DESKTOP_SHELL | Work as Weston desktop-shell |
| USE_VIRTUAL_KEYBOARD | Use Virtual Keyboard (only when you use `DESKTOP_SHELL`) |
| USE_GLES3 | Use OpenGLES3 instead of OpenGLES2 |

## 4. Running Flutter app

### Install Flutter SDK
See also: [Desktop support for Flutter](https://flutter.dev/desktop)

```Shell
$ git clone https://github.com/flutter/flutter
$ sudo mv flutter /opt/
$ export PATH=$PATH:/opt/flutter/bin
$ flutter config --enable-linux-desktop
$ flutter doctor
```

Please note that you must use the same version (channel) that you built Flutter embedder for. I recommend that you use the latest version of the master channel for both the SDK and Flutter Engine.
See also: [Building Flutter Engine embedder](./BUILDING-ENGINE-EMBEDDER.md)

### Build Flutter app

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

### Run Flutter app

#### Run with Wayland backend

Wayland compositor such as Weston must be running before running the program.

```Shell
$ ./flutter-client ./sample/build/linux/x64/release/bundle
```

#### Supplement

You can switch quickly between debug / profile / release modes for the Flutter app without replacing `libflutter_engine.so` by using `LD_LIBRARY_PATH` when you run the Flutter app.

```Shell
$ LD_LIBRARY_PATH=<path_to_engine> ./flutter-client <path_to_flutter_project_bundle>

# e.g. Run in debug mode
$ LD_LIBRARY_PATH=/usr/lib/flutter_engine/debug/ ./flutter-client ./sample/build/linux/x64/debug/bundle

# e.g. Run in profile mode
$ LD_LIBRARY_PATH=/usr/lib/flutter_engine/profile/ ./flutter-client ./sample/build/linux/x64/profile/bundle

# e.g. Run in release mode
$ LD_LIBRARY_PATH=/usr/lib/flutter_engine/release/ ./flutter-client ./sample/build/linux/x64/release/bundle
```

#### Run with DRM backend

You need to switch from GUI which is running X11 or Wayland to the Character User Interface (CUI). In addition, `FLUTTER_DRM_DEVICE` must be set properly. The default value is `/dev/dri/card0`.

```Shell
$ Ctrl + Alt + F3 # Switching to CUI
$ sudo FLUTTER_DRM_DEVICE="/dev/dri/card1" ./flutter-drm-backend ./sample/build/linux/x64/release/bundle
```

If you want to switch back from CUI to GUI, run `Ctrl + Alt + F2` keys in a terminal.

##### Note
You need to run this program by a user who has the permission to access the input devices(/dev/input/xxx), if you use the DRM backend. Generally, it is a root user or a user who belongs to an input group.

### Debugging
You can do debugging Flutter apps. Please see: [How to debug Flutter apps](./DEBUGGING.md)

## 5. Settings of weston.ini file (Only when you use Weston desktop-shell)

Sets the following parameters when this embedder works as a desktop-shell on Weston. Sample file can be found [examples/config/weston.ini](./examples/config/weston.ini). See also `man weston.ini`.

### shell section

Specifies the path to the binary file to start as the shell when Weston starts.

| Field | Description |
| ------------- | ------------- |
| client | ${path to the binary}/flutter-desktop-shell |

### extended section

An extended section for this embedder. The entries that can appear in this section are:

| Field | Description |
| ------------- | ------------- |
| show-cursor | Set whether to show mouse cursor (boolean) |
| flutter-project-path | Set an absolute path or relative path from the binary file to Flutter project path (string) |
