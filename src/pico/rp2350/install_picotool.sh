#!/usr/bin/env bash
# Build Raspberry Pi picotool with libusb and install to ~/.local/bin.
# Does not need sudo. USB device nodes still need udev (install_picotool_udev.sh).
set -euo pipefail

PREFIX="${PREFIX:-$HOME/.local}"
SDK="${PICO_SDK_PATH:-/home/v/01_projects/13_hardware/pico/pico-sdk}"
SRC="${PICOTOOL_SRC:-}"
BUILD="${PICOTOOL_BUILD:-/home/v/01_projects/13_hardware/pico/picotool-usb-build}"
BIN="$PREFIX/bin/picotool"

have_usb_picotool() {
  local p="$1"
  [[ -x "$p" ]] || return 1
  ! "$p" version 2>&1 | grep -q "compiled without USB support"
}

if have_usb_picotool "$BIN"; then
  echo "picotool already has USB: $BIN"
  "$BIN" version
  exit 0
fi

if [[ -z "$SRC" ]]; then
  for cand in \
    /home/v/01_projects/13_hardware/pico/picotool \
    /home/v/01_projects/13_hardware/pico/gameboy/build/_deps/picotool-src
  do
    if [[ -f "$cand/CMakeLists.txt" ]]; then
      SRC="$cand"
      break
    fi
  done
fi

if [[ -z "$SRC" || ! -f "$SRC/CMakeLists.txt" ]]; then
  SRC=/home/v/01_projects/13_hardware/pico/picotool
  git clone --depth 1 https://github.com/raspberrypi/picotool.git "$SRC"
fi

if [[ ! -f "$SDK/pico_sdk_init.cmake" ]]; then
  echo "FAILED: PICO_SDK_PATH missing pico_sdk_init.cmake (tried $SDK)" >&2
  exit 1
fi

export PICO_SDK_PATH="$SDK"
mkdir -p "$BUILD" "$PREFIX/bin"
cmake -S "$SRC" -B "$BUILD" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$PREFIX" \
  -DPICO_SDK_PATH="$SDK" \
  -DPICOTOOL_NO_LIBUSB=OFF \
  -G Ninja
cmake --build "$BUILD" -j"$(nproc)"
cmake --install "$BUILD"

if ! have_usb_picotool "$BIN"; then
  echo "FAILED: installed picotool still has no USB ($BIN)" >&2
  "$BIN" version >&2 || true
  exit 1
fi

echo "installed $BIN"
"$BIN" version
echo "USB device access needs udev once: sudo $(dirname "$0")/install_picotool_udev.sh"
exit 0
