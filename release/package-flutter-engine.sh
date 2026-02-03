#!/usr/bin/env bash

set -ex

# Example)
#
#./package-flutter-engine.sh --arch=arm64 --build-dir=build-embedder --artifact-dir=. --output-dir=$(pwd)
#

BUILD_DIR=${BUILD_DIR:-"build-embedder"}
TARGET_ARCH=${TARGET_ARCH:-"x64"}
ARTIFACTS_PATH=${ARTIFACTS_PATH:-"."}
OUTPUT_DIR=${OUTPUT_DIR:-$(pwd)}

for i; do
	case $i in
		--arch=*)
			TARGET_ARCH="${i#*=}"
			;;
		--build-dir=*)
			BUILD_DIR="${i#*=}"
			;;
		--artifact-dir=*)
			ARTIFACTS_PATH="${i#*=}"
			;;
		--output-dir=*)
			OUTPUT_DIR="${i#*=}"
			;;
	esac
done

ENGINE_ROOT=${BUILD_DIR}/flutter/engine

if [ ${TARGET_ARCH} = "arm64" ]; then
	DEBUG_OUTDIR="linux_debug_unopt_arm64"
	PROFILE_OUTDIR="linux_profile_arm64"
	RELEASE_OUTDIR="linux_release_arm64"
else
	DEBUG_OUTDIR="host_debug_unopt"
	PROFILE_OUTDIR="host_profile"
	RELEASE_OUTDIR="host_release"
fi

## common
mkdir deploy_package
if [ ${TARGET_ARCH} = "x64" ]; then
	mkdir deploy_package/icu
	cp ${ENGINE_ROOT}/src/out/host_debug_unopt/icudtl.dat deploy_package/icu/
	unzip ${ARTIFACTS_PATH}/../common/embedder-common.zip -d deploy_package/

	cd deploy_package
	zip -r elinux-common.zip *
	cd ..
	mv deploy_package/elinux-common.zip ${OUTPUT_DIR}
	rm -rf deploy_package/*
fi


## debug mode
cp ${ENGINE_ROOT}/src/out/${DEBUG_OUTDIR}/libflutter_engine.so deploy_package/
unzip ${ARTIFACTS_PATH}/embedder-${TARGET_ARCH}-debug.zip -d deploy_package/
zip -jr ${OUTPUT_DIR}/elinux-${TARGET_ARCH}-debug.zip deploy_package/*
rm -rf deploy_package/*


## profile mode
cp ${ENGINE_ROOT}/src/out/${PROFILE_OUTDIR}/libflutter_engine.so deploy_package/
mkdir deploy_package/linux-x64
if [ ${TARGET_ARCH} = "x64" ]; then
	cp ${ENGINE_ROOT}/src/out/${PROFILE_OUTDIR}/gen_snapshot deploy_package/linux-x64/
else
	cp ${ENGINE_ROOT}/src/out/${PROFILE_OUTDIR}/clang_x64/gen_snapshot deploy_package/linux-x64/
fi

pushd deploy_package
zip -r elinux-${TARGET_ARCH}-profile.zip *
mv elinux-${TARGET_ARCH}-profile.zip ${OUTPUT_DIR}
popd

rm -rf deploy_package/*

## release mode
cp ${ENGINE_ROOT}/src/out/${RELEASE_OUTDIR}/libflutter_engine.so deploy_package/
mkdir deploy_package/linux-x64
if [ ${TARGET_ARCH} = "x64" ]; then
	cp ${ENGINE_ROOT}/src/out/${RELEASE_OUTDIR}/gen_snapshot deploy_package/linux-x64/
else
	cp ${ENGINE_ROOT}/src/out/${RELEASE_OUTDIR}/clang_x64/gen_snapshot deploy_package/linux-x64/
fi
unzip ${ARTIFACTS_PATH}/embedder-${TARGET_ARCH}-release.zip -d deploy_package/

pushd deploy_package
zip -r elinux-${TARGET_ARCH}-release.zip *
mv elinux-${TARGET_ARCH}-release.zip ${OUTPUT_DIR}
popd


rm -rf deploy_package
