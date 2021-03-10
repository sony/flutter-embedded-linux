# How to build Flutter Engine embedder

See also:
- [Custom Flutter Engine Embedders](https://github.com/flutter/flutter/wiki/Custom-Flutter-Engine-Embedders)
- [Custom Flutter Engine Embedding in AOT Mode](https://github.com/flutter/flutter/wiki/Custom-Flutter-Engine-Embedding-in-AOT-Mode)

## 1. Install build tools

```
$ git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
$ export PATH=$PATH:$(pwd)/depot_tools
```

### Python 2

Python 2 is required to build. If you default installation is Python 3 you could workaround this by using virtualenv:

```Shell
$ virtualenv .env -p python2
$ source .env/bin/activate
```

See also: https://github.com/dart-lang/sdk/wiki/Building#python-2

## 2. Create .gclient file

### When using the latest version

```yaml
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

### When using a specific version

You can check the current engine version (commit id / SHA):
- [master channel](https://raw.githubusercontent.com/flutter/flutter/master/bin/internal/engine.version)
- [dev channel](https://raw.githubusercontent.com/flutter/flutter/dev/bin/internal/engine.version)
- [beta channel](https://raw.githubusercontent.com/flutter/flutter/beta/bin/internal/engine.version)
- [stable channel](https://raw.githubusercontent.com/flutter/flutter/stable/bin/internal/engine.version)

```yaml
solutions = [
  {
    "managed": False,
    "name": "src/flutter",
    "url": "https://github.com/flutter/engine.git@FLUTTER_ENGINE",
    "custom_deps": {},
    "deps_file": "DEPS",
    "safesync_url": "",
  },
]
```
Note: Replace `FLUTTER_ENGINE` with the commid  it of the Flutter engine you want to use.

## 3. Get source files

```Shell
$ gclient sync
```

## 4. Build embedder

```Shell
$ cd src
```

### for arm64 targets with debug mode

```Shell
$ ./flutter/tools/gn --target-os linux --linux-cpu arm64 --runtime-mode debug --unoptimized --embedder-for-target
$ ninja -C out/linux_debug_unopt_arm64
```

### for arm64 targets with profile mode

```Shell
$ ./flutter/tools/gn --target-os linux --linux-cpu arm64 --runtime-mode profile --no-lto --embedder-for-target
$ ninja -C out/linux_profile_arm64
```

### for arm64 targets with release mode

```Shell
$ ./flutter/tools/gn --target-os linux --linux-cpu arm64 --runtime-mode release --embedder-for-target
$ ninja -C out/linux_release_arm64
```

### for x64 targets with debug mode

```Shell
$ ./flutter/tools/gn --runtime-mode debug --unoptimized --embedder-for-target
$ ninja -C out/host_debug_unopt
```

### for x64 targets with profile mode

```Shell
$ ./flutter/tools/gn --runtime-mode profile --no-lto --embedder-for-target
$ ninja -C out/host_profile
```

### for x64 targets with release mode

```Shell
$ ./flutter/tools/gn --runtime-mode release --embedder-for-target
$ ninja -C out/host_release
```

## 5. Install embedder library

```Shell
$ sudo cp ./out/linux_{your selected target and mode}/libflutter_engine.so /usr/lib
```
