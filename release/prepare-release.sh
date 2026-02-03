#!/usr/bin/bash

error() {
	printf "%s\n" "$@" >&2
	exit 1
}

print_tag() {
	local dir="$1"

	(
		cd "$dir" || exit
		[ -e embedder.version ] || error "Missing embedder.version, not a release dir?"
		[ -e elinux-arm64-debug.zip ] || error "release incomplete: missing elinux-arm64-debug.zip"
		[ -e elinux-arm64-profile.zip ] || error "release incomplete: missing elinux-arm64-profile.zip"
		[ -e elinux-arm64-release.zip ] || error "release incomplete: missing elinux-arm64-release.zip"
		[ -e elinux-common.zip ] || error "release incomplete: missing elinux-common.zip"
		[ -e elinux-x64-debug.zip ] || error "release incomplete: missing elinux-x64-debug.zip"
		[ -e elinux-x64-profile.zip ] || error "release incomplete: missing elinux-x64-profile.zip"
		[ -e elinux-x64-release.zip ] || error "release incomplete: missing elinux-x64-release.zip"
		[ -e embedder.channel ] || error "release incomplete: missing embedder.channel"
		[ -e flutter.version ] || error "release incomplete: missing flutter.version"
		[ -e sdk.version ] || error "release incomplete: missing sdk.version"

		# check zip files content are sane-ish? not-empty, valid zips?
		# For now manually check sizes are in the correct ballpark in below output..

		ENGINE_VER=$(cat embedder.version)
		TAG=${ENGINE_VER:0:10}

		cat <<EOF


*****************
$dir
*****************

command to create tag:
==================
git fetch origin
git tag $TAG origin/master
git push origin $TAG
==================

github release:
https://github.com/flutter-elinux/flutter-embedded-linux/releases/new?tag=$TAG

title: engine-artifacts-$TAG

text for the github release:
==================
### Artifacts for flutter-elinux

The corresponding official Flutter version is as follow:

#### Flutter tool version:

$(cat sdk.version)

#### Flutter Engine version:

$ENGINE_VER
===================

Files to include:
EOF
		ls -lh elinux-arm64-debug.zip elinux-arm64-profile.zip elinux-arm64-release.zip elinux-common.zip elinux-x64-debug.zip elinux-x64-profile.zip elinux-x64-release.zip

		if [ "$dir" != "output-$TAG" ]; then
			cat <<EOF

Rename dir:
mv "$dir" output-$TAG
EOF
		fi
	)
}

for dir in "$@"; do
	print_tag "$dir"
done
