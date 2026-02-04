This directory contains scripts used to build the releases.

To run with docker:

```
# build container
docker build -t flutter-build .
# build flutter engine and flutter-elinux variants
docker run -ti --rm --user "$(id -u):$(id -g)" \
    -v "$PWD":/flutter -w /flutter \
    --env HOME=/flutter/home flutter-build \
    ./build-all.sh [build-all options]
# check build is complete and output instructions to make release
./prepare-release.sh output
```

build-all.sh by default builds the latest stable release, but a flutter
commit id can be passed with --flutter-rev=xxxxxx

The flutter engine takes time to build 6 times (debug profile release * x64 arm64),
so expect this to take about 1h on a fast machine, and a few hours on anything slower.

The first build will fetch various source directories including flutter
source dependency tree, following builds will re-use what's available and
only update as necessary depending on build-all.sh options.
