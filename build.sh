#!/usr/bin/env zsh

set -euo pipefail

build_dir="build/release_macos"
install_dir="distr"

cmake -B "$build_dir" -DCMAKE_INSTALL_PREFIX="$install_dir"
cmake --build "$build_dir"

mkdir -p "$install_dir"
cp -r "$build_dir/dancetools" "$install_dir/"
