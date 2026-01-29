#!/usr/bin/env zsh
set -euo pipefail

install_dir="distr"
bin_name="dancetools"

# arm64 build
build_dir_arm="build/release_macos_arm64"
cmake -S . -B "$build_dir_arm" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_PREFIX_PATH="./third-party/arm64-osx"
cmake --build "$build_dir_arm" --config Release

# x86_64 build
build_dir_x64="build/release_macos_x86_64"
cmake -S . -B "$build_dir_x64" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES=x86_64 \
  -DCMAKE_PREFIX_PATH="./third-party/x64-osx"
cmake --build "$build_dir_x64" --config Release

# universal merge (arm64 + x86_64)
mkdir -p "$install_dir"

arm_bin="$build_dir_arm/$bin_name"
x64_bin="$build_dir_x64/$bin_name"
universal_bin="$install_dir/$(basename "$bin_name")"

# create universal binary
lipo -create -output "$universal_bin" "$arm_bin" "$x64_bin"
file "$universal_bin"