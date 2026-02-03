#!/usr/bin/env bash

set -ex

CHANNEL=${CHANNEL:-"stable"}
BUILD_DIR=${BUILD_DIR:-"build-embedder"}

for i; do
	case $i in
		--build-dir=*)
			BUILD_DIR="${i#*=}"
			;;
		--channel=*)
			CHANNEL="${i#*=}"
			;;
		--flutter-rev=*)
			FLUTTER_REV="${i#*=}"
			CHANNEL=""
			;;
		*)
			echo "bad option $1"
			exit 1
			;;
	esac
done

install_depot_tools() {
	# Get gclient
	if [ ! -d depot_tools ]; then
		git clone --filter blob:none https://chromium.googlesource.com/chromium/tools/depot_tools.git
	fi
	export PATH="$PATH:$PWD/depot_tools"
}

# required for git checkout in flutter repo
install_depot_tools

mkdir -p "$BUILD_DIR"

if [ -n "$CHANNEL" ]; then
	[ -z "$FLUTTER_REV" ] || exit 1
	# Get flutter engine version
	curl -OL "https://raw.githubusercontent.com/flutter/flutter/$CHANNEL/bin/internal/engine.version"
	mv engine.version "$BUILD_DIR/embedder.version"
	ENGINE_VERSION=$(cat "$BUILD_DIR/embedder.version")

	# Save channel
	echo "$CHANNEL" > "$BUILD_DIR/embedder.channel"

	if [ -n "${USE_FLUTTER_SRC_CACHE}" ] && [ -d "$BUILD_DIR/flutter" ] ; then
		echo "Using cached flutter source"
	else
		# Get flutter source code
		if ! [ -d "$BUILD_DIR/flutter" ]; then
			git clone --filter blob:none https://github.com/flutter/flutter.git "$BUILD_DIR/flutter"
		elif ! git -C "$BUILD_DIR/flutter" rev-parse "$ENGINE_VERSION" 2>/dev/null; then
			git -C "$BUILD_DIR/flutter" fetch
		fi
		git -C "$BUILD_DIR/flutter" checkout "$ENGINE_VERSION"
	fi

	# Get SDK version
	"$BUILD_DIR/flutter/bin/flutter" channel "$CHANNEL"
	"$BUILD_DIR/flutter/bin/flutter" upgrade
	"$BUILD_DIR/flutter/bin/flutter" --version > "$BUILD_DIR/sdk.version"
	git -C "$BUILD_DIR/flutter" rev-parse HEAD > "$BUILD_DIR/flutter.version"
	git -C "$BUILD_DIR/flutter" describe --tags >> "$BUILD_DIR/flutter.version"

elif [ -n "$FLUTTER_REV" ]; then

	# Get flutter source code
	if ! [ -d "$BUILD_DIR/flutter" ]; then
		git clone --filter blob:none https://github.com/flutter/flutter.git "$BUILD_DIR/flutter"
	elif ! git -C "$BUILD_DIR/flutter" rev-parse "$FLUTTER_REV" 2>/dev/null; then
		git -C "$BUILD_DIR/flutter" fetch
	fi
	git -C "$BUILD_DIR/flutter" checkout "$FLUTTER_REV"

	echo "$FLUTTER_REV" > "$BUILD_DIR/flutter.version"
	git -C "$BUILD_DIR/flutter" describe --tags >> "$BUILD_DIR/flutter.version"
	cat "$BUILD_DIR/flutter/bin/internal/engine.version" > "$BUILD_DIR/embedder.version"
	ENGINE_VERSION=$(cat "$BUILD_DIR/embedder.version")
	echo user-branch > "$BUILD_DIR/embedder.channel"

	# run twice to fetch engine first (and not have fetch messages in sdk.version file)
	"$BUILD_DIR/flutter/bin/flutter" --version
	"$BUILD_DIR/flutter/bin/flutter" --version > "$BUILD_DIR/sdk.version"
	# fix repo url
	sed -i -e 's#unknown source#https://github.com/flutter/flutter.git#' "$BUILD_DIR/sdk.version"
else
	echo "set either --channel or --engine"
	exit 1
fi


# Confirm version
SDK_VERSION=$(cat "$BUILD_DIR/sdk.version")
SHORT_VERSION=${ENGINE_VERSION:0:10}
if [[ ${SDK_VERSION} != *"${SHORT_VERSION}"* ]]; then
	exit 1
fi
