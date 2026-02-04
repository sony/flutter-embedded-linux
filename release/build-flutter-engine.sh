#!/usr/bin/env bash

set -ex

# Example)
#
#./build-flutter-engine.sh --revision=$(cat embedder.version) --arch=arm64 --build-dir=build-embedder
#

BUILD_DIR=${BUILD_DIR:-"build-embedder"}
TARGET_ARCH=${TARGET_ARCH:-"x64"}
REVISION=${REVISION:-""}
OUTPUT_DIR="out"
REPO_URL=https://github.com/flutter/flutter.git

for i; do
	case $i in
		--revision=*)
			REVISION="${i#*=}"
			;;
		--arch=*)
			TARGET_ARCH="${i#*=}"
			;;
		--build-dir=*)
			BUILD_DIR="${i#*=}"
			;;
	esac
done



# check if need to build.
if [ -z "${REVISION}" ]; then
	echo "Warning: No revision specified."
fi

# Builds Flutter Engine

install_depot_tools() {
	# Get gclient
	if [ ! -d depot_tools ]; then
		git clone --filter blob:none https://chromium.googlesource.com/chromium/tools/depot_tools.git
	fi
	export PATH=$PATH:$(pwd)/depot_tools
}

get_flutter_source() {
	echo "Flutter Engine version: "${REVISION}

	# already checked out?
	if [ -n "$REVISION" ] && [ "$(git -C flutter rev-parse HEAD "$REVISION" 2>/dev/null | uniq | wc -l)" = 1 ]; then
		return
	fi

	# Clone flutter repo
	if [ ! -d flutter ]; then
		git clone --filter blob:none ${REPO_URL}
	elif  [ -z "$REVISION" ] || ! git -C flutter rev-parse "$REVISION" 2>/dev/null; then
		git -C flutter fetch
	fi

	if [ -n "${REVISION}" ]; then
		git -C flutter checkout "${REVISION}"
	fi
}

set_flutter_path() {
	export PATH=$PATH:$(pwd)/flutter/engine/src/flutter/bin

	# patch: https://github.com/flutter/flutter/issues/163487
	export PATH=$(pwd)/flutter/engine/src/flutter/third_party/ninja:$PATH
	export PATH=$(pwd)/flutter/engine/src/flutter/third_party/depot_tools/ninja:$PATH
}

gclient_sync() {
	# gclient sync
	pushd flutter

	if [ -n "${REVISION}" ]; then
		URL="${REPO_URL}@${REVISION}"
	else
		URL="${REPO_URL}"
	fi

	if  [ -e .gclient_sync_ok ] && grep -q "$URL" .gclient; then
		popd
		return
	fi

	rm -f .gclient_sync_ok
	cat <<EOF | tee .gclient
solutions = [
    {
	"managed": False,
	"name": ".",
	"url": "${URL}",
	"custom_deps": {},
	"deps_file": "DEPS",
	"safesync_url": "",
	"custom_vars": {
	    "download_android_deps": False,
	    "download_windows_deps": False,
    },
},
]
EOF

	# remove previous build artifacts...
	rm -rf engine/src/out

	# retry up to 3 times
	gclient sync -D --reset || gclient sync -D --reset || gclient sync -D --reset
	touch .gclient_sync_ok

	popd
}

build_flutter_engine() {

# Build engine
pushd flutter/engine/src

ARCH_OPTION=""
if [ ${TARGET_ARCH} = "arm64" ]; then
	ARCH_OPTION="--target-os linux --linux-cpu arm64 --arm-float-abi hard"
	DEBUG_OUTDIR="linux_debug_unopt_arm64"
	PROFILE_OUTDIR="linux_profile_arm64"
	RELEASE_OUTDIR="linux_release_arm64"
else
	DEBUG_OUTDIR="host_debug_unopt"
	PROFILE_OUTDIR="host_profile"
	RELEASE_OUTDIR="host_release"
fi


# debug mode
./flutter/tools/gn --runtime-mode debug ${ARCH_OPTION} --unoptimized --embedder-for-target --disable-desktop-embeddings --no-build-embedder-examples --enable-fontconfig --no-goma
ninja -C out/${DEBUG_OUTDIR}

# profile mode
./flutter/tools/gn --runtime-mode profile ${ARCH_OPTION} --no-lto --embedder-for-target --disable-desktop-embeddings --no-build-embedder-examples --enable-fontconfig --no-goma
ninja -C out/${PROFILE_OUTDIR}

# release mode
./flutter/tools/gn --runtime-mode release ${ARCH_OPTION} --embedder-for-target --disable-desktop-embeddings --no-build-embedder-examples --enable-fontconfig --no-goma
ninja -C out/${RELEASE_OUTDIR}

popd
}


mkdir -p ${BUILD_DIR}
pushd ${BUILD_DIR}

install_depot_tools
get_flutter_source
set_flutter_path
gclient_sync
build_flutter_engine

popd
