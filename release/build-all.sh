#!/usr/bin/env bash

set -ex

# Example)
#
# ./build-all.sh --channel=stable
# ./build-all.sh --channel=stable --build-dir=my-build --output-dir=my-output
# ./build-all.sh --flutter-rev=19074d12f7eaf6a8180cd4036a430c1d76de904e

CHANNEL=${CHANNEL:-"stable"}
BUILD_DIR=${BUILD_DIR:-"build-embedder"}
OUTPUT_DIR=${OUTPUT_DIR:-"$PWD/output"}
RUNTIME_MODE=${RUNTIME_MODE:-"release"}

for i; do
	case $i in
		--channel=*)
			CHANNEL="${i#*=}"
			;;
		--flutter-rev=*)
			FLUTTER_REV="${i#*=}"
			CHANNEL=
			;;
		--build-dir=*)
			BUILD_DIR="${i#*=}"
			;;
		--output-dir=*)
			OUTPUT_DIR="${i#*=}"
			;;
		--runtime-mode=*)
			RUNTIME_MODE="${i#*=}"
			;;
		*)
			echo "Bad option $i"
			exit 1
			;;
	esac
done

echo "=========================================="
echo "Flutter eLinux Full Build Script"
echo "=========================================="
echo "Channel: ${CHANNEL}$FLUTTER_REV"
echo "Build directory: ${BUILD_DIR}"
echo "Output directory: ${OUTPUT_DIR}"
echo "Runtime mode: ${RUNTIME_MODE}"
echo "=========================================="

# Create output directory
mkdir -p ${OUTPUT_DIR}

# rm -f "$BUILD_DIR/flutter/.gclient"


if [ -z "${SKIP_GET_FLUTTER_VERSION}" ]; then
	# Step 1: Get Flutter versions
	echo ""
	echo "=========================================="
	echo "Step 1: Getting Flutter versions..."
	echo "=========================================="
	./get-flutter-versions.sh ${CHANNEL:+--channel=${CHANNEL}} ${FLUTTER_REV:+--flutter-rev=$FLUTTER_REV} --build-dir=${BUILD_DIR}
fi

# Read engine version
ENGINE_VERSION=$(cat ${BUILD_DIR}/embedder.version)
echo "Engine version: ${ENGINE_VERSION}"

if [ -z "${SKIP_BUILD_FLUTTER_ENGINE_X64}" ]; then
	# Step 2: Build Flutter Engine for x64
	echo ""
	echo "=========================================="
	echo "Step 2: Building Flutter Engine for x64..."
	echo "=========================================="
	./build-flutter-engine.sh \
		--revision=${ENGINE_VERSION} \
		--arch=x64 \
		--build-dir=${BUILD_DIR}
fi

if [ -z "${SKIP_BUILD_FLUTTER_ENGINE_ARM64}" ]; then
	# Step 3: Build Flutter Engine for arm64
	echo ""
	echo "=========================================="
	echo "Step 3: Building Flutter Engine for arm64..."
	echo "=========================================="
	./build-flutter-engine.sh \
		--revision=${ENGINE_VERSION} \
		--arch=arm64 \
		--build-dir=${BUILD_DIR}
fi

WORK_DIR_X64="work-x64"
if [ -z "${SKIP_BUILD_FLUTTER_EMBEDDER_X64}" ]; then
	# Step 4: Build Flutter Embedder for x64
	echo ""
	echo "=========================================="
	echo "Step 4: Building Flutter Embedder for x64..."
	echo "=========================================="
	mkdir -p ${WORK_DIR_X64}
	cd ${WORK_DIR_X64}

	mkdir -p usr/lib
	rm -rf artifacts-x64

	../build-flutter-embedder.sh \
		--arch=x64 \
		--output-dir=artifacts-x64 \
		--engine-dir=../${BUILD_DIR}/flutter/engine \
		--runtime-mode=${RUNTIME_MODE} \
		--install-dir=$(realpath ./usr/lib)

	cd ..
fi

WORK_DIR_ARM64="work-arm64"
if [ -z "${SKIP_BUILD_FLUTTER_EMBEDDER_ARM64}" ]; then
	# Step 5: Build Flutter Embedder for arm64
	echo ""
	echo "=========================================="
	echo "Step 5: Building Flutter Embedder for arm64..."
	echo "=========================================="
	mkdir -p ${WORK_DIR_ARM64}
	cp -f cross-toolchain-aarch64.cmake "$WORK_DIR_ARM64"
	cd ${WORK_DIR_ARM64}

	mkdir -p usr/lib
	rm -rf artifacts-arm64

	../build-flutter-embedder.sh \
		--arch=arm64 \
		--output-dir=artifacts-arm64 \
		--engine-dir=../${BUILD_DIR}/flutter/engine \
		--runtime-mode=${RUNTIME_MODE} \
		--install-dir=$(realpath ./usr/lib)

	cd ..
fi

if [ -z "${SKIP_PACKAGE_FLUTTER_ENGINE_X64}" ]; then
	# Step 6: Package Flutter Engine for x64
	echo ""
	echo "=========================================="
	echo "Step 6: Packaging Flutter Engine for x64..."
	echo "=========================================="
	./package-flutter-engine.sh \
		--arch=x64 \
		--build-dir=${BUILD_DIR} \
		--artifact-dir=${WORK_DIR_X64}/artifacts-x64/x64 \
		--output-dir=${OUTPUT_DIR}
fi

if [ -z "${SKIP_PACKAGE_FLUTTER_ENGINE_ARM64}" ]; then
	# Step 7: Package Flutter Engine for arm64
	echo ""
	echo "=========================================="
	echo "Step 7: Packaging Flutter Engine for arm64..."
	echo "=========================================="
	./package-flutter-engine.sh \
		--arch=arm64 \
		--build-dir=${BUILD_DIR} \
		--artifact-dir=${WORK_DIR_ARM64}/artifacts-arm64/arm64 \
		--output-dir=${OUTPUT_DIR}
fi

# Step 8: Copy embedder.version to output
echo ""
echo "=========================================="
echo "Step 8: Copying metadata..."
echo "=========================================="
cp ${BUILD_DIR}/embedder.version ${OUTPUT_DIR}/
cp ${BUILD_DIR}/embedder.channel ${OUTPUT_DIR}/
cp ${BUILD_DIR}/flutter.version ${OUTPUT_DIR}/
cp ${BUILD_DIR}/sdk.version ${OUTPUT_DIR}/

# Step 9: List generated files
echo ""
echo "=========================================="
echo "Build completed successfully!"
echo "=========================================="
echo ""
echo "Generated files in ${OUTPUT_DIR}/:"
ls -lh ${OUTPUT_DIR}/*.zip ${OUTPUT_DIR}/*.version 2>/dev/null || true

echo ""
echo "Engine version: ${ENGINE_VERSION}"
echo "Channel: ${CHANNEL}"
echo ""
echo "Output packages:"
echo "  - elinux-common.zip"
echo "  - elinux-x64-debug.zip"
echo "  - elinux-x64-profile.zip"
echo "  - elinux-x64-release.zip"
echo "  - elinux-arm64-debug.zip"
echo "  - elinux-arm64-profile.zip"
echo "  - elinux-arm64-release.zip"
echo ""
echo "All artifacts are ready for release!"
echo "=========================================="

# Cleanup option (uncomment if you want to clean up work directories)
# echo ""
# echo "Cleaning up work directories..."
# rm -rf ${WORK_DIR_X64} ${WORK_DIR_ARM64}
# echo "Cleanup completed."
