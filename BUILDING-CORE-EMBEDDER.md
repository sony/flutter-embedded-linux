# How to build core custom embedder

See also: [Custom Flutter Engine Embedders](https://github.com/flutter/flutter/wiki/Custom-Flutter-Engine-Embedders), [Custom Flutter Engine Embedding in AOT Mode](https://github.com/flutter/flutter/wiki/Custom-Flutter-Engine-Embedding-in-AOT-Mode)

## 1. Install build tools

```
$ git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
$ export PATH=$PATH:$(pwd)/depot_tools
```

## 2. Create .gclinet file

```
solutions = [
  {
    "managed": False,
    "name": "src/flutter",
    "url": "https://github.com/flutter/engine.git",
    "custom_deps": {},
    "deps_file": "DEPS",
    "safesync_url": "",
  },
]
```

## 3. Get source files

```
$ gclient sync
```

## 4. Build embedder

```
$ cd src
```

### for arm64 targets with debug mode

```
$ ./flutter/tools/gn --target-os linux --linux-cpu arm64 --runtime-mode debug --unoptimized --embedder-for-target
$ ninja -C out/linux_debug_unopt_arm64
```

### for arm64 targets with profile mode

```
$ ./flutter/tools/gn --target-os linux --linux-cpu arm64 --runtime-mode profile --no-lto --embedder-for-target
$ ninja -C out/linux_profile_arm64
```

### for arm64 targets with release mode

```
$ ./flutter/tools/gn --target-os linux --linux-cpu arm64 --runtime-mode release --embedder-for-target
$ ninja -C out/linux_release_arm64
```

### for x64 targets with debug mode

```
$ ./flutter/tools/gn --runtime-mode debug --unoptimized --embedder-for-target
$ ninja -C out/host_debug_unopt
```

### for x64 targets with profile mode

```
$ ./flutter/tools/gn --runtime-mode profile --no-lto --embedder-for-target
$ ninja -C out/host_profile
```

#### for x64 targets with release mode

```
$ ./flutter/tools/gn --runtime-mode release --embedder-for-target
$ ninja -C out/host_release
```

## 5. Install embedder library

```
$ sudo cp ./out/linux_{your selected target and mode}/libflutter_engine.so /usr/lib
```
