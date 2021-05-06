# meta-flutter for Yocto Project
Recipe file examples for Yocto Project Yocto Project. See also: https://docs.yoctoproject.org/

## Setup environment
### Create a base environment
Use the Yocto project version `dunfell` :
```Shell
$ mkdir ~/workspace-dunfell && cd ~/workspace-dunfell
$ git clone git://git.yoctoproject.org/poky.git -b dunfell
$ git clone https://github.com/kraj/meta-clang -b dunfell
$ git clone https://github.com/sony/flutter-embedded-linux.git
```

Setting environment variables :
```Shell
$ source poky/oe-init-build-env build
```

Add meta-clang layer into conf/bblayers.conf :
```Shell
$ bitbake-layers add-layer ../meta-clang
```

Add meta-flutter layer into conf/bblayers.conf :
```Shell
$ bitbake-layers add-layer ../flutter-embedded-linux/meta-flutter
```

Build the target file system image :
```Shell
$ bitbake core-image-weston
```

## Building Flutter
### Wayland backend 
```Shell
$ bitbake flutter-wayland-client
```

### DRM-GBM backend
`libsystemd` is required to build this backend. Putting the following in your conf/local.conf: 
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

## SDK toolchain
If you use the SDK, please follow the steps below.  
Putting the following in your conf/local.conf :
```
CLANGSDK = "1"
```
See also: [Adding clang in generated SDK toolchain](https://github.com/kraj/meta-clang/blob/master/README.md#adding-clang-in-generated-sdk-toolchain)

The SDK on yocto is built with the following command : 
```Shell
$ bitbake core-image-weston -c populate_sdk
```
See also: [SDK building an sdk installer](https://www.yoctoproject.org/docs/2.1/sdk-manual/sdk-manual.html#sdk-building-an-sdk-installer)

Install the SDK :  
e.g. aarch64
```Shell
$ ./tmp/deploy/sdk/poky-glibc-x86_64-core-image-weston-aarch64-qemuarm64-toolchain-3.1.7.sh
```

This `cross-toolchain.cmake` is a sample of aarch64. Please set the variable in your environment to match the location of your SDK installation :  
e.g. Build for Wayland backend  
```Shell
$ cd flutter-embedded-linux
$ mkdir build && cd build
$ cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/cross-toolchain.cmake -DUSER_PROJECT_PATH=examples/flutter-wayland-clinet ..
$ make
```
## Note
The build would fail because of the lack of flutter-engine recipe. We are still creating it. Please wait.  
