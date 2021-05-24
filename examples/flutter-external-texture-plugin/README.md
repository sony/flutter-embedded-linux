# Overview

This is the example of External Texture Plugin. Source files are implemented based on [this reference](https://github.com/jnschulze/flutter-playground/tree/master/windows_texture_test).

## Building (Wayland backend stand-alone application)

```Shell
$ mkdir build
$ cd build
$ cmake -DUSER_PROJECT_PATH=examples/flutter-external-texture-plugin ..
$ cmake --build .
```

## Dart sample code for Flutter app

```Dart
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'dart:async';

void main() {
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  int _textureId = 0;

  @override
  void initState() {
    super.initState();
    initialize().then((value) => setState(() {}));
  }

  Future<void> initialize() async {
    late Completer<void> creatingCompleter;
    try {
      creatingCompleter = Completer<void>();
      var channel = const MethodChannel('external_texture_test');
      final reply =
          await channel.invokeMapMethod<String, dynamic>('initialize');
      if (reply != null) {
        _textureId = reply['textureId'];
      }
    } on PlatformException catch (e) {}

    creatingCompleter.complete();
    return creatingCompleter.future;
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        backgroundColor: Colors.blue,
        appBar: AppBar(
          title: const Text('External Texture Plugin sample'),
        ),
        body: Center(
          child: AspectRatio(
              aspectRatio: 16 / 9, child: Texture(textureId: _textureId)),
        ),
      ),
    );
  }
}
```

### Note

Needs the Flutter SDK version 1.20 and above.
