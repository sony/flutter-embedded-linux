# meta-flutter for Yocto Project
Recipe file examples for Yocto Project. See also: https://docs.yoctoproject.org/

## Overview
As a premise, this document uses Yocto Version(dunfel), Yocto Image(core-image-weston) and Machine Selection(qemuarm64) as an example.

## 1. Setup environment

### 1.1. Building Yocto Image

### Downloading the Poky and meta-clang, meta-flutter source code
```Shell
$ git clone git://git.yoctoproject.org/poky.git -b dunfell
$ git clone https://github.com/kraj/meta-clang -b dunfell
$ git clone https://github.com/sony/flutter-embedded-linux.git
```

### Preparing the build environment
Inside the poky directory, there is a script named oe-init-build-env, which should be used to set up the build environment.
```Shell
$ source poky/oe-init-build-env build
```

### local.conf
You need to select a specific machine to target the build with. Selecting the following in your conf/local.conf :
```
MACHINE ?= "qemuarm64"
```

### bblayers.conf
Add meta-clang layer into conf/bblayers.conf :
```Shell
$ bitbake-layers add-layer ../meta-clang
```
Add meta-flutter layer into conf/bblayers.conf :
```Shell
$ bitbake-layers add-layer ../flutter-embedded-linux/meta-flutter
```

### Building a target image
```Shell
$ bitbake core-image-weston
```


### 1.2. Building Yocto SDK

### Include clang in Yocto SDK
Putting the following in your conf/local.conf :
```
CLANGSDK = "1"
```
See also: [Adding clang in generated SDK toolchain](https://github.com/kraj/meta-clang/blob/master/README.md#adding-clang-in-generated-sdk-toolchain)

### Building a target Yocto SDK
```Shell
$ bitbake core-image-weston -c populate_sdk
```
See also: [SDK building an sdk installer](https://www.yoctoproject.org/docs/2.1/sdk-manual/sdk-manual.html#sdk-building-an-sdk-installer)

### Installing Yocto SDK on host PC
```Shell
$ ./tmp/deploy/sdk/poky-glibc-x86_64-core-image-weston-aarch64-qemuarm64-toolchain-3.1.7.sh
```


## 2. Cross compile using bitbake

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

## 3. Cross compile using the Yocto SDK
You need to create a toolchain file to cross compile using the Yocto SDK for aarch64 on x64 hosts. [toolcahin-yocto-template.cmake](../cmake/cross-toolchain-aarch64-yocto-template.cmake) is the templete file for aarch64 toolchain. Also, you need to modify <path_to_user_target_sysroot> and <path_to_user_toolchain> appropriately for your environment if you want to use the template file.
```Shell
$ cmake -DUSER_PROJECT_PATH=<path_to_user_project> -DCMAKE_TOOLCHAIN_FILE=<toolcahin-yocto-template-file>
```


## Note
The build would fail because of the lack of flutter-engine recipe. We are still creating it. Please wait.  
