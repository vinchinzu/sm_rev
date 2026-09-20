#!/usr/bin/env bash
# One-time host setup so picotool can force a running SDK app into BOOTSEL
# without holding the button or unplugging. Re-execs sudo if needed.
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
RULES_SRC="$HERE/60-picotool.rules"
RULES_DST="/etc/udev/rules.d/60-picotool.rules"

if [[ ! -f "$RULES_SRC" ]]; then
  echo "FAILED: missing $RULES_SRC" >&2
  exit 1
fi

if [[ "$(id -u)" -ne 0 ]]; then
  exec sudo -- "$0" "$@"
fi

install -m 0644 "$RULES_SRC" "$RULES_DST"
udevadm control --reload-rules
udevadm trigger --action=add --subsystem-match=usb --attr-match=idVendor=2e8a || true
udevadm trigger --action=add --subsystem-match=tty || true

# Apply now so this session does not need an unplug. udev MODE=0666 is the
# persistent path; chmod covers the already-enumerated Pico.
chmod_pico_nodes() {
  local d vid ser bus dev node tty
  for d in /sys/bus/usb/devices/*; do
    vid="$(cat "$d/idVendor" 2>/dev/null || true)"
    [[ "$vid" == "2e8a" ]] || continue
    ser="$(cat "$d/serial" 2>/dev/null || true)"
    bus="$(cat "$d/busnum" 2>/dev/null || true)"
    dev="$(cat "$d/devnum" 2>/dev/null || true)"
    if [[ -n "$bus" && -n "$dev" ]]; then
      node="$(printf '/dev/bus/usb/%03d/%03d' "$bus" "$dev")"
      if [[ -e "$node" ]]; then
        chmod 0666 "$node" || true
        echo "usb $node serial=${ser:-?} $(stat -c '%a %U:%G' "$node")"
      fi
    fi
    for tty in "$d"/*/tty/ttyACM*; do
      [[ -e "$tty" ]] || continue
      node="/dev/$(basename "$tty")"
      if [[ -e "$node" ]]; then
        chmod 0666 "$node" || true
        echo "cdc $node serial=${ser:-?} $(stat -c '%a %U:%G' "$node")"
      fi
    done
  done
}

chmod_pico_nodes

echo "installed $RULES_DST"
echo "picotool load -f should work without BOOTSEL. Flash with: make pico-flash"
exit 0
