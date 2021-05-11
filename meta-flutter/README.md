# meta-flutter for Yocto Project
Recipe file examples for Yocto Project. In this README, we explain how to build for Arm64 using `core-image-weston` which is one of Yocto Images, and `dunfell` which is one of LTS Yocto versions. See also: https://docs.yoctoproject.org/


## Setup environment
There are two ways to build using Yocto. One is a build using bitbake and the other is a build using Yocto SDK.

### Building Yocto
Downloading `Poky` and `meta-clang`, `meta-flutter` source code:
```Shell
$ git clone git://git.yoctoproject.org/poky.git -b dunfell
$ git clone https://github.com/kraj/meta-clang -b dunfell
$ git clone https://github.com/sony/flutter-embedded-linux.git
```

Setup the build environment using `oe-init-build-env` script in Poky.
```Shell
$ source poky/oe-init-build-env build
```

Set your target machine in your `conf/local.conf`:
```
MACHINE ?= "qemuarm64"
```

Add meta-clang layer into `conf/bblayers.conf`.
```Shell
$ bitbake-layers add-layer ../meta-clang
```

Add meta-flutter layer into `conf/bblayers.conf`.
```Shell
$ bitbake-layers add-layer ../flutter-embedded-linux/meta-flutter
```

### Building Yocto SDK (Only when using cross-building with Yocto SDK)
Add the following in your `conf/local.conf`:
```
CLANGSDK = "1"
```
See also: [Adding clang in generated SDK toolchain](https://github.com/kraj/meta-clang/blob/master/README.md#adding-clang-in-generated-sdk-toolchain)

Build Yocto SDK for cross-building.
```Shell
$ bitbake core-image-weston -c populate_sdk
```
See also: [SDK building an sdk installer](https://www.yoctoproject.org/docs/2.1/sdk-manual/sdk-manual.html#sdk-building-an-sdk-installer)

Install Yocto SDK.
```Shell
$ ./tmp/deploy/sdk/poky-glibc-x86_64-core-image-weston-aarch64-qemuarm64-toolchain-3.1.7.sh
```

## Cross-building with bitbake
### Flutter Engine (libflutter_engine.so)
Build target is fixed to Linux and Arm64, and Flutter Engine version is also fixed in the recipe file.

#### Flutter Engine version
```
ENGINE_VERSION ?= "f5b97d0b23a3905e9b5b69aa873afcb52f550aaf"
```

When creating a Flutter project, you will need to use the following version of the Flutter SDK.  
`Flutter 2.3.0-1.0.pre.269`

If you want to change the version of the Flutter engine, change <engine_version> to the appropriate version of the Flutter SDK and add the following to `conf/local.conf`:
```
ENGINE_VERSION_pn-flutter-engine = "<engine_version>"
```

Flutter Engine is built with release mode by default. If you want to change the build mode, you can change it to add the following in your `conf/local.conf`:
#### Flutter Engine mode
```
# e.g. debug mode
PACKAGECONFIG_pn-flutter-engine = "debug-mode"
```

### Wayland backend
```Shell
$ bitbake flutter-wayland-client
```

### DRM-GBM backend
`libsystemd` is required to build this backend. Put the following in your `conf/local.conf`: 
```
DESTRO_FEATURES_append = " systemd"
```
See also: [Using systemd for the Main Image and Using SysVinit for the Rescue Image](https://www.yoctoproject.org/docs/current/mega-manual/mega-manual.html#using-systemd-for-the-main-image-and-using-sysvinit-for-the-rescue-image)

```Shell
$ bitbake flutter-drm-gbm-backend
```

### DRM-EGLStream backend
You need to install libsystemd in the same way with the DRM-GBM backend.

```Shell
$ bitbake flutter-drm-eglstream-backend
```

## Cross-building with Yocto SDK
Setup the cross-building toolchain environment using a script that you built and installed.
```Shell
$ source /opt/poky/3.1.7/environment-setup-aarch64-poky-linux
```

Set the following environment vars to cross-build using clang.
```Shell
$ export CC=${CLANGCC}
$ export CXX=${CLANGCXX}
```

After doing that, you can build the embedder as normal like self-building on hosts. It means you don't need to be aware of cross-building. See: [3.1. Self-build](../doc/README.md)
