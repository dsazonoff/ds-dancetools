#!/usr/bin/env zsh

set -euo pipefail

./bootstrap/bootstrap.sh --triplet=arm64-osx
./bootstrap/bootstrap.sh --triplet=x64-osx
