#!/usr/bin/env bash
# Flash sm_rev_pico2.uf2 onto serial 3973D48FD625B2E8 only.
#
# picotool load -f uses the running SDK USB stdio vendor reset.
# No BOOTSEL button, no unplug.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
HERE="$(cd "$(dirname "$0")" && pwd)"
UF2="${1:-$ROOT/build/pico2/sm_rev_pico2.uf2}"
BOARD_SERIAL="3973D48FD625B2E8"
OTHER_SERIAL="C89554A4009B3D21"
LOG="${FLASH_LOG:-/tmp/pico2_sm_rev_flash.log}"

find_picotool() {
  local p
  if [[ -n "${PICOTOOL:-}" ]]; then
    printf '%s\n' "$PICOTOOL"
    return 0
  fi
  for p in \
    "$HOME/.local/bin/picotool" \
    "$(command -v picotool 2>/dev/null || true)" \
    /home/v/01_projects/13_hardware/pico/picotool-usb-build/picotool
  do
    [[ -n "$p" && -x "$p" ]] || continue
    printf '%s\n' "$p"
    return 0
  done
  return 1
}

picotool_has_usb() {
  local p="$1"
  [[ -x "$p" ]] || return 1
  ! "$p" version 2>&1 | grep -q "compiled without USB support"
}

usb_serials() {
  local d vid pid ser
  for d in /sys/bus/usb/devices/*; do
    vid="$(cat "$d/idVendor" 2>/dev/null || true)"
    pid="$(cat "$d/idProduct" 2>/dev/null || true)"
    ser="$(cat "$d/serial" 2>/dev/null || true)"
    [[ -n "$vid" && -n "$pid" && -n "$ser" ]] || continue
    printf '%s %s %s %s\n' "$vid" "$pid" "$ser" "$(basename "$d")"
  done
}

refuse_other() {
  if usb_serials | awk '{print $3}' | grep -qx "$OTHER_SERIAL"; then
    echo "FAILED: other Pico 2 $OTHER_SERIAL is present; unplug it" | tee -a "$LOG"
    exit 1
  fi
}

in_bootsel() {
  usb_serials | awk -v s="$BOARD_SERIAL" '$1=="2e8a" && $2=="000f" && $3==s {found=1} END{exit !found}'
}

in_runtime() {
  usb_serials | awk -v s="$BOARD_SERIAL" '$1=="2e8a" && $2!="000f" && $3==s {found=1} END{exit !found}'
}

usb_node_writable() {
  local d vid pid ser bus dev node
  for d in /sys/bus/usb/devices/*; do
    vid="$(cat "$d/idVendor" 2>/dev/null || true)"
    pid="$(cat "$d/idProduct" 2>/dev/null || true)"
    ser="$(cat "$d/serial" 2>/dev/null || true)"
    [[ "$vid" == "2e8a" && "$ser" == "$BOARD_SERIAL" ]] || continue
    bus="$(cat "$d/busnum" 2>/dev/null || true)"
    dev="$(cat "$d/devnum" 2>/dev/null || true)"
    [[ -n "$bus" && -n "$dev" ]] || continue
    node="$(printf '/dev/bus/usb/%03d/%03d' "$bus" "$dev")"
    [[ -w "$node" ]] && return 0
  done
  return 1
}

print_udev_help() {
  echo "picotool cannot open USB (device node is not writable)."
  echo "One-time host fix, then re-run (no BOOTSEL, no unplug):"
  echo "  $HERE/install_picotool_udev.sh"
}

{
  echo "=== $(date -Iseconds) flash $UF2 ==="
  usb_serials
} >"$LOG"

if [[ ! -f "$UF2" ]]; then
  echo "FAILED: missing $UF2"
  exit 1
fi

refuse_other

PT="$(find_picotool || true)"
if [[ -z "$PT" ]]; then
  echo "FAILED: picotool not on PATH. Build one with: $HERE/install_picotool.sh"
  exit 1
fi
if ! picotool_has_usb "$PT"; then
  echo "FAILED: $PT was built without USB. Build one with: $HERE/install_picotool.sh"
  exit 1
fi
echo "picotool: $PT"

if ! in_runtime && ! in_bootsel; then
  echo "FAILED: Pico serial $BOARD_SERIAL is not on USB"
  usb_serials | tee -a "$LOG"
  exit 1
fi

if ! usb_node_writable; then
  print_udev_help | tee -a "$LOG"
  exit 1
fi

# device-selection (-f, --ser) must come after the filename; -f first
# makes clipp treat later --ser as unexpected.
echo "picotool load -x $UF2 --ser $BOARD_SERIAL -f"
if "$PT" load -x "$UF2" --ser "$BOARD_SERIAL" -f; then
  echo "DONE: picotool flashed $BOARD_SERIAL"
  usb_serials >>"$LOG"
  exit 0
fi

echo "FAILED: picotool load -f did not flash $BOARD_SERIAL" | tee -a "$LOG"
exit 1
