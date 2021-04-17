# Documents

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

#### Only when using Wayland backend
- libwayland
- wayland-protocols (for generating xdg-shell source files)

```Shell
$ sudo apt install libwayland-dev wayland-protocols
```

#### Only when using weston desktop-shell
- weston (>=6.0.1, see: [#3](https://github.com/sony/flutter-embedded-linux/issues/3))

```Shell
$ sudo apt install weston
```

#### Only when using DRM backend
- libdrm
- libgbm
- libinput
- libudev
- libsystemd

```Shell
$ sudo apt install libdrm-dev libgbm-dev libinput-dev libudev-dev libsystemd-dev
```

#### Only when using x11 backend
- x11

```Shell
$ sudo apt install libx11-dev
```

#### Install Flutter Engine library

This embedder requres `libflutter_engine.so` (Flutter embedder library). You need to install `libflutter_engine.so` in `/usr/lib` to build. See: [Building Flutter Engine embedder](./building-engine-embedder.md)

Or you can download a specific pre-built Flutter Engine from Google's infra by the following steps, but it's limited to **debug mode** and **x64** targets.

Step 1) Check the version (SHA) of the channel you want to use.
- [master channel](https://raw.githubusercontent.com/flutter/flutter/master/bin/internal/engine.version)
- [dev channel](https://raw.githubusercontent.com/flutter/flutter/dev/bin/internal/engine.version)
- [beta channel](https://raw.githubusercontent.com/flutter/flutter/beta/bin/internal/engine.version)
- [stable channel](https://raw.githubusercontent.com/flutter/flutter/stable/bin/internal/engine.version)

You can also get the version from `${path_to_flutter_sdk_install}/flutter/bin/internal/engine.version` of the Flutter SDK which you are currently using.

Step 2) Download Flutter Engine embedder library. Note that replace `FLUTTER_ENGINE` with the SHA of the Flutter engine you want to use.
```Shell
$ curl -O https://storage.googleapis.com/flutter_infra/flutter/FLUTTER_ENGINE/linux-x64/linux-x64-embedder
```

Step 3) Install the library. Note that the downloaded library is only **debug mode** and for **x64** targets. 
```Shell
$ unzip ./linux-x64-embedder
$ cp ./libflutter_engine.so <path_to_cmake_build_directory>
```

## 2. Examples
There are sample projects in [`examples`](../examples) directory. You can also comunicate with Dart code by using the plugin APIs with the same specifications as with Flutter desktops for Windows.

## 3. Building

### Build for Wayland backend (Stand-alone Wayland app)

```Shell
$ mkdir build
$ cd build
$ cmake -DUSER_PROJECT_PATH=examples/flutter-wayland-client ..
$ cmake --build .
```

### Build for Wayland backend (Weston desktop-shell)
This binary will run as a desktop-shell by setting `weston.ini` when Weston starts. See [Settings of weston.ini file](#6-settings-of-westonini-file-only-when-using-weston-desktop-shell).

```Shell
$ mkdir build
$ cd build
$ cmake -DUSER_PROJECT_PATH=examples/flutter-weston-desktop-shell ..
$ cmake --build .
```

### Build for DRM backend

#### Use GBM

```Shell
$ mkdir build
$ cd build
$ cmake -DUSER_PROJECT_PATH=examples/flutter-drm-gbm-backend ..
$ cmake --build .
```

#### Use EGLStream

```Shell
$ mkdir build
$ cd build
$ cmake -DUSER_PROJECT_PATH=examples/flutter-drm-eglstream-backend ..
$ cmake --build .
```

### Build for x11 backend (Stand-alone X11 app)

Basically, the x11 backend is just only for debugging and developing Flutter apps on desktops. And it's still being implemented now.

```Shell
$ mkdir build
$ cd build
$ cmake -DUSER_PROJECT_PATH=examples/flutter-x11-client ..
$ cmake --build .
```

### How to debug the embedder
You need to build the embedder with `CMAKE_BUILD_TYPE=Debug` option if you want to debug the embedder. Using this option, you can get gather logs and debug them with debuggers such as gdb / lldb.

```Shell
$ cmake -DUSER_PROJECT_PATH=<path_to_user_project> -DCMAKE_BUILD_TYPE=Debug ..
```

### User configuration parameters (CMAKE options)

Please edit `cmake/user_config.cmake` file.

| Option | Description |
| ------------- | ------------- |
| BACKEND_TYPE | Select WAYLAND, DRM-GBM, DRM-EGLSTREAM, or X11 as the display backend type (The default setting is WAYLAND) |
| DESKTOP_SHELL | Work as Weston desktop-shell |
| USE_VIRTUAL_KEYBOARD | Use Virtual Keyboard (only when using `DESKTOP_SHELL`) |
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
See also: [Building Flutter Engine embedder](./building-engine-embedder.md)

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
$ sudo FLUTTER_DRM_DEVICE="/dev/dri/card1" <binary_file_name> ./sample/build/linux/x64/release/bundle
```

If you want to switch back from CUI to GUI, run `Ctrl + Alt + F2` keys in a terminal.

##### Note
You need to run this program by a user who has the permission to access the input devices(/dev/input/xxx), if you use the DRM backend. Generally, it is a root user or a user who belongs to an input group.

## 5. Debugging Flutter apps
You can do debugging Flutter apps. Please see: [How to debug Flutter apps](./debugging.md)

## 6. Settings of weston.ini file (Only when using Weston desktop-shell)

Sets the following parameters when this embedder works as a desktop-shell on Weston. Sample file can be found [examples/config/weston.ini](../examples/config/weston.ini). See also `man weston.ini`.

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
