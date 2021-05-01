# meta-flutter for Yocto Project
Recipe file examples for Yocto Project Yocto Project. See also: https://docs.yoctoproject.org/

## Setup environment
### meta-clang
If needed, add meta-clang layer into conf/bblayers.conf:
```Shell
$ git clone https://github.com/kraj/meta-clang -b dunfell
$ bitbake-layers add-layer meta-clang
```

### meta-flutter
```Shell
$ git clone https://github.com/sony/flutter-embedded-linux.git
$ bitbake-layers add-layer ./flutter-embedded-linux/meta-flutter/
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

## Note
The build would fail because of the lack of flutter-engine recipe. We are still creating it. Please wait.  
