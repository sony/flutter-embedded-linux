# How to debug Flutter apps
It is possible to do most things, including source-level debugging, profiling and hot reload when you debug Flutter apps on this embedder. Naturally, you need to **use `libflutter_engine.so` built with debug mode**.

## Building with debug mode

You need to build the embedder using `CMAKE_BUILD_TYPE` option.

```Shell
$ cmake -DUSER_PROJECT_PATH=examples/flutter-x11-client -DCMAKE_BUILD_TYPE=Debug ..
$ cmake --build .
```

## Getting the Observatory port

You will find the following message in the console when you run Flutter apps. You need to attach a debugger to this port when you want to debug it. **Note that the URI changes every time after launching**.

```Shell
flutter: Observatory listening on http://127.0.0.1:40409/k8IUol2dnPI=/
```
## Debugging

### Command line

```Shell
$ flutter attach --device-id=flutter-tester --debug-uri http://127.0.0.1:40409/k8IUol2dnPI=/
```

### How to use command line options in the Dart VM
You can use the environment variables such as `FLUTTER_ENGINE_SWITCHES`, `FLUTTER_ENGINE_SWITCH_*` to use Dart VM command line options. If you want to fix always same observatory RUI, you can use the following command.

```Shell
$ FLUTTER_ENGINE_SWITCHES=2 \
    FLUTTER_ENGINE_SWITCH_1="observatory-port=12345"  \
    FLUTTER_ENGINE_SWITCH_2="disable-service-auth-codes" \
    ./flutter-x11-clinet sample/build/linux/x64/debug/bundle
flutter: Observatory listening on http://127.0.0.1:12345/
```

### VS Code

Create a [launch.json file](https://code.visualstudio.com/docs/editor/debugging#_launch-configurations) like the following.

```Json
{
  "version": "0.2.0"
  "configurations": [
    {
      "type": "dart",
      "name": "Flutter Embedded Linux Attach",
      "request": "attach",
      "deviceId": "flutter-tester",
      "observatoryUri": "http://127.0.0.1:40409/k8IUol2dnPI=/"
    }
  ]
}
```
