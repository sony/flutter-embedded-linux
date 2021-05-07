# meta-flutter for Yocto Project
Recipe file examples for Yocto Project. See also: https://docs.yoctoproject.org/

## How to build Yocto Image

### Clone Repositories
```Shell
# e.g. Yocto project version dunfell
$ git clone git://git.yoctoproject.org/poky.git -b dunfell

# e.g. Yocto project version dunfell
$ git clone https://github.com/kraj/meta-clang -b dunfell

$ git clone https://github.com/sony/flutter-embedded-linux.git
```

### OpenEmbedded build environment
```Shell
$ source poky/oe-init-build-env build
```

### Add meta-clang layer
Add meta-clang layer into conf/bblayers.conf :
```Shell
$ bitbake-layers add-layer ../meta-clang
```

### Add meta-flutter layer
Add meta-flutter layer into conf/bblayers.conf :
```Shell
$ bitbake-layers add-layer ../flutter-embedded-linux/meta-flutter
```

### Build an image
```Shell
# e.g. core-image-weston
$ bitbake core-image-weston
```


## How to build Yocto SDK

### Include clang in Yocto SDK
Putting the following in your conf/local.conf :
```
CLANGSDK = "1"
```
See also: [Adding clang in generated SDK toolchain](https://github.com/kraj/meta-clang/blob/master/README.md#adding-clang-in-generated-sdk-toolchain)

### Generates Yocto SDK
```Shell
# e.g. core-image-weston
$ bitbake core-image-weston -c populate_sdk
```
See also: [SDK building an sdk installer](https://www.yoctoproject.org/docs/2.1/sdk-manual/sdk-manual.html#sdk-building-an-sdk-installer)

### How to install Yocto SDK on host PC
```Shell
# e.g. aarch64
$ ./tmp/deploy/sdk/poky-glibc-x86_64-core-image-weston-aarch64-qemuarm64-toolchain-3.1.7.sh
```


## How to cross-building with bitbake

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

## How to cross-building with Yocto SDK
You need to build the embedder with `CMAKE_TOOLCHAIN_FILE=<toolcahin-yocto-template-file>` option if you want to cross-building with Yocto SDK. This [toolcahin-yocto-template-file](../cmake/cross-toolchain-aarch64-yocto-template.cmake) is a template of aarch64.
```Shell
$ cmake -DUSER_PROJECT_PATH=<path_to_user_project> -DCMAKE_TOOLCHAIN_FILE=<toolcahin-yocto-template-file>
```


## Note
The build would fail because of the lack of flutter-engine recipe. We are still creating it. Please wait.  
