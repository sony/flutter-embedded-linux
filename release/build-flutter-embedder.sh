#!/usr/bin/env bash

set -ex

# Example)
#
# ./build-flutter-embedder.sh --arch=arm64 --output-dir=artifacts --engine-dir=../build-embedder/flutter/engine
#

TARGET_ARCH=${TARGET_ARCH:-"x64"}
OUTPUT_DIR=${OUTPUT_DIR:-"artifacts"}
EMBEDDER_REPO=${EMBEDDER_REPO:-"https://github.com/flutter-elinux/flutter-embedded-linux.git"}
ENGINE_DIR=${ENGINE_DIR:-""}
RUNTIME_MODE=${RUNTIME_MODE:-"release"}
INSTALL_DIR=${INSTALL_DIR:-"/usr/lib"}

export CMAKE_BUILD_PARALLEL_LEVEL=24

for i; do
	case $i in
		--arch=*)
			TARGET_ARCH="${i#*=}"
			;;
		--output-dir=*)
			OUTPUT_DIR="${i#*=}"
			;;
		--embedder-repo=*)
			EMBEDDER_REPO="${i#*=}"
			;;
		--engine-dir=*)
			ENGINE_DIR="${i#*=}"
			;;
		--runtime-mode=*)
			RUNTIME_MODE="${i#*=}"
			;;
		--install-dir=*)
			INSTALL_DIR="${i#*=}"
	esac
done

# Validate ENGINE_DIR
if [ -z "${ENGINE_DIR}" ]; then
	echo "Error: ENGINE_DIR is not set. Please specify --engine-dir=<path>"
	echo "Example: --engine-dir=../build-embedder/flutter/engine"
	exit 1
fi

if [ ! -d "${ENGINE_DIR}" ]; then
	echo "Error: ENGINE_DIR does not exist: ${ENGINE_DIR}"
	exit 1
fi

ABS_INSTALL_DIR=$(realpath ${INSTALL_DIR})

echo "Engine directory: ${ENGINE_DIR}"
echo "Target architecture: ${TARGET_ARCH}"
echo "Runtime mode: ${RUNTIME_MODE}"

# Determine output directories based on architecture
if [ ${TARGET_ARCH} = "arm64" ]; then
	ARCH_ARGS="-DCMAKE_TOOLCHAIN_FILE=$PWD/cross-toolchain-aarch64.cmake"
	DEBUG_OUTDIR="linux_debug_unopt_arm64"
	PROFILE_OUTDIR="linux_profile_arm64"
	RELEASE_OUTDIR="linux_release_arm64"
else
	DEBUG_OUTDIR="host_debug_unopt"
	PROFILE_OUTDIR="host_profile"
	RELEASE_OUTDIR="host_release"
fi

# Install libflutter_engine.so from built engine
install_flutter_engine() {

	echo "Installing Flutter Engine to ${ABS_INSTALL_DIR}"
	mkdir -p ${ABS_INSTALL_DIR}

	# Copy libflutter_engine.so files
	cp ${ENGINE_DIR}/src/out/${DEBUG_OUTDIR}/libflutter_engine.so ${ABS_INSTALL_DIR}/libflutter_engine-debug.so
	cp ${ENGINE_DIR}/src/out/${PROFILE_OUTDIR}/libflutter_engine.so ${ABS_INSTALL_DIR}/libflutter_engine-profile.so
	cp ${ENGINE_DIR}/src/out/${RELEASE_OUTDIR}/libflutter_engine.so ${ABS_INSTALL_DIR}/libflutter_engine-release.so

	# Create symlink based on runtime mode
	ln -sf ./libflutter_engine-${RUNTIME_MODE}.so ${ABS_INSTALL_DIR}/libflutter_engine.so

	echo "Flutter Engine installed successfully."
}

# Install build dependencies (uncomment if needed)
# install_dependencies() {
#     apt install -y zip equivs clang libgles2-mesa-dev wayland-protocols libegl1-mesa-dev
#     apt install -y libdrm-dev libgbm-dev libinput-dev libudev-dev libsystemd-dev
#     apt install -y libxkbcommon-dev libwayland-dev
# }

# Creates artifacts directory
mkdir -p ${OUTPUT_DIR}/${TARGET_ARCH}/debug
mkdir -p ${OUTPUT_DIR}/${TARGET_ARCH}/release
mkdir -p ${OUTPUT_DIR}/common

# Install libflutter_engine.so from the built engine
install_flutter_engine

# Get source files
if ! [ -d flutter-embedded-linux ]; then
	git clone --filter blob:none ${EMBEDDER_REPO}
else
	git -C flutter-embedded-linux fetch
	git -C flutter-embedded-linux reset --hard origin/master
fi
cd flutter-embedded-linux
git rev-parse --short HEAD > ../${OUTPUT_DIR}/embedder.version
cat ../${OUTPUT_DIR}/embedder.version
mkdir -p build
cd build

# Build wayland (for debug mode)
cp ${ABS_INSTALL_DIR}/libflutter_engine.so .
cmake $ARCH_ARGS -DBUILD_ELINUX_SO=ON -DBACKEND_TYPE=WAYLAND -DCMAKE_BUILD_TYPE=Release -DENABLE_ELINUX_EMBEDDER_LOG=ON -DFLUTTER_RELEASE=OFF ..
cmake --build .
cp libflutter_elinux_wayland.so ../../${OUTPUT_DIR}/${TARGET_ARCH}/debug
rm -rf *

# Build wayland (for release mode)
cp ${ABS_INSTALL_DIR}/libflutter_engine.so .
cmake $ARCH_ARGS -DBUILD_ELINUX_SO=ON -DBACKEND_TYPE=WAYLAND -DCMAKE_BUILD_TYPE=Release -DENABLE_ELINUX_EMBEDDER_LOG=OFF -DFLUTTER_RELEASE=ON ..
cmake --build .
cp libflutter_elinux_wayland.so ../../${OUTPUT_DIR}/${TARGET_ARCH}/release
rm -rf *

# Build x11 (for debug mode)
cp ${ABS_INSTALL_DIR}/libflutter_engine.so .
cmake $ARCH_ARGS -DBUILD_ELINUX_SO=ON -DBACKEND_TYPE=X11 -DCMAKE_BUILD_TYPE=Release -DENABLE_ELINUX_EMBEDDER_LOG=ON -DFLUTTER_RELEASE=OFF ..
cmake --build .
cp libflutter_elinux_x11.so ../../${OUTPUT_DIR}/${TARGET_ARCH}/debug
rm -rf *

# Build x11 (for release mode)
cp ${ABS_INSTALL_DIR}/libflutter_engine.so .
cmake $ARCH_ARGS -DBUILD_ELINUX_SO=ON -DBACKEND_TYPE=X11 -DCMAKE_BUILD_TYPE=Release -DENABLE_ELINUX_EMBEDDER_LOG=OFF -DFLUTTER_RELEASE=ON ..
cmake --build .
cp libflutter_elinux_x11.so ../../${OUTPUT_DIR}/${TARGET_ARCH}/release
rm -rf *

# Build gbm (for debug mode)
cp ${ABS_INSTALL_DIR}/libflutter_engine.so .
cmake $ARCH_ARGS -DBUILD_ELINUX_SO=ON -DBACKEND_TYPE=DRM-GBM -DCMAKE_BUILD_TYPE=Release -DENABLE_ELINUX_EMBEDDER_LOG=ON -DFLUTTER_RELEASE=OFF ..
cmake --build .
cp libflutter_elinux_gbm.so ../../${OUTPUT_DIR}/${TARGET_ARCH}/debug
rm -rf *

# Build gbm (for release mode)
cp ${ABS_INSTALL_DIR}/libflutter_engine.so .
cmake $ARCH_ARGS -DBUILD_ELINUX_SO=ON -DBACKEND_TYPE=DRM-GBM -DCMAKE_BUILD_TYPE=Release -DENABLE_ELINUX_EMBEDDER_LOG=OFF -DFLUTTER_RELEASE=ON ..
cmake --build .
cp libflutter_elinux_gbm.so ../../${OUTPUT_DIR}/${TARGET_ARCH}/release
rm -rf *

# Build eglstream (for debug mode)
cp ${ABS_INSTALL_DIR}/libflutter_engine.so .
cmake $ARCH_ARGS -DBUILD_ELINUX_SO=ON -DBACKEND_TYPE=DRM-EGLSTREAM -DCMAKE_BUILD_TYPE=Release -DENABLE_ELINUX_EMBEDDER_LOG=ON -DFLUTTER_RELEASE=OFF ..
cmake --build .
cp libflutter_elinux_eglstream.so ../../${OUTPUT_DIR}/${TARGET_ARCH}/debug
rm -rf *

# Build eglstream (for release mode)
cp ${ABS_INSTALL_DIR}/libflutter_engine.so .
cmake $ARCH_ARGS -DBUILD_ELINUX_SO=ON -DBACKEND_TYPE=DRM-EGLSTREAM -DCMAKE_BUILD_TYPE=Release -DENABLE_ELINUX_EMBEDDER_LOG=OFF -DFLUTTER_RELEASE=ON ..
cmake --build .
cp libflutter_elinux_eglstream.so ../../${OUTPUT_DIR}/${TARGET_ARCH}/release
rm -rf *

cd ../..

# Creates release packages
cd ${OUTPUT_DIR}/${TARGET_ARCH}
zip -jr embedder-${TARGET_ARCH}-debug.zip debug/*
zip -jr embedder-${TARGET_ARCH}-release.zip release/*
cd ../..

# Gathers common files
cp -rf flutter-embedded-linux/src/client_wrapper ${OUTPUT_DIR}/common
cp -rf flutter-embedded-linux/src/flutter/shell/platform/common/client_wrapper ${OUTPUT_DIR}/common
cp -rf flutter-embedded-linux/src/flutter/shell/platform/common/public/* ${OUTPUT_DIR}/common
cp -rf flutter-embedded-linux/src/flutter/shell/platform/linux_embedded/public/* ${OUTPUT_DIR}/common

cd ${OUTPUT_DIR}/common
mv client_wrapper cpp_client_wrapper
zip -r embedder-common.zip *
cd ../..

echo ""
echo "Build completed successfully."
echo "Artifacts are in ${OUTPUT_DIR}/"
