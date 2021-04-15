# meta-flutter for Yocto Project

Recipe file examples for Yocto Project Yocto Project. See also: https://docs.yoctoproject.org/

## Building Flutter

Build for Wayland backend  
```Shell
$ bitbake flutter-client
```

Build for DRM backend  
Adding the systemd to your build. See also: https://www.yoctoproject.org/docs/current/mega-manual/mega-manual.html#using-systemd-for-the-main-image-and-using-sysvinit-for-the-rescue-image  
via local.conf  
`DESTRO_FEATURES_append = " systemd"`

```Shell
$ bitbake flutter-drm-backend
```

## Note
Build does not pass because of engine/flutter-engine ongoing.
