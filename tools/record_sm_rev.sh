#!/usr/bin/env bash
set -Eeuo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

state_path="${1:-saves/save1.sav}"
output_path="${2:-recordings/sm_rev_$(date +%Y%m%d_%H%M%S).mp4}"

bin_path="${SM_REV_BIN:-./sm_rev}"
backend="${BACKEND:-auto}"
window_title="${WINDOW_TITLE:-SuperMet}"
window_id="${WINDOW_ID:-}"
fps="${FPS:-30}"
duration="${DURATION:-180}"
max_mb="${MAX_MB:-9.5}"
scale_height="${SCALE_HEIGHT:-0}"
game_wait="${GAME_WAIT:-2}"
launch_game="${LAUNCH_GAME:-1}"

game_pid=""
rec_pid=""
watcher_pid=""
recorder_kind=""
ffmpeg_control_fd=""
ffmpeg_control_fifo=""

die() {
  printf 'record_sm_rev: %s\n' "$*" >&2
  exit 1
}

require_tool() {
  command -v "$1" >/dev/null 2>&1 || die "missing required tool: $1"
}

is_running() {
  local pid="$1"
  [[ -n "$pid" ]] && kill -0 "$pid" >/dev/null 2>&1
}

stop_recorder() {
  if is_running "$watcher_pid"; then
    kill -TERM "$watcher_pid" >/dev/null 2>&1 || true
    wait "$watcher_pid" || true
  fi
  watcher_pid=""

  if ! is_running "$rec_pid"; then
    rec_pid=""
    return 0
  fi

  case "$recorder_kind" in
    gsr)
      kill -INT "$rec_pid" >/dev/null 2>&1 || true
      ;;
    ffmpeg)
      if [[ -n "$ffmpeg_control_fd" ]]; then
        printf 'q' >&"$ffmpeg_control_fd" || true
        exec {ffmpeg_control_fd}>&- || true
        ffmpeg_control_fd=""
      else
        kill -INT "$rec_pid" >/dev/null 2>&1 || true
      fi
      ;;
    *)
      kill -TERM "$rec_pid" >/dev/null 2>&1 || true
      ;;
  esac

  for _ in {1..80}; do
    is_running "$rec_pid" || break
    sleep 0.1
  done
  if is_running "$rec_pid"; then
    kill -TERM "$rec_pid" >/dev/null 2>&1 || true
  fi
  wait "$rec_pid" || true
  rec_pid=""
}

cleanup() {
  local status=$?
  trap - EXIT INT TERM
  stop_recorder
  if is_running "$game_pid"; then
    kill -TERM "$game_pid" >/dev/null 2>&1 || true
    wait "$game_pid" || true
  fi
  [[ -n "$ffmpeg_control_fifo" ]] && rm -f "$ffmpeg_control_fifo"
  exit "$status"
}

trap cleanup EXIT INT TERM

[[ -x "$bin_path" ]] || die "missing executable: $bin_path"
[[ -f "$state_path" ]] || die "missing save state: $state_path"
require_tool awk
require_tool setsid

mkdir -p "$(dirname "$output_path")"

if [[ "$backend" == "auto" ]]; then
  if [[ "${XDG_SESSION_TYPE:-}" == "wayland" || -n "${WAYLAND_DISPLAY:-}" ]]; then
    backend="wayland-gsr"
  else
    backend="x11-ffmpeg"
  fi
fi

if [[ "$launch_game" == "1" ]]; then
  printf 'record_sm_rev: launching %s from %s\n' "$bin_path" "$state_path"
  setsid "$bin_path" --load-state "$state_path" &
  game_pid="$!"
  sleep "$game_wait"
  if ! is_running "$game_pid"; then
    wait "$game_pid" || true
    die "game exited before recording started"
  fi
fi

bitrate_k="240"
if [[ "$duration" != "0" ]]; then
  bitrate_k="$(awk -v mb="$max_mb" -v seconds="$duration" '
    BEGIN {
      kbps = (mb * 8000 * 0.68) / seconds;
      if (kbps < 80) kbps = 80;
      if (kbps > 650) kbps = 650;
      printf "%d", kbps;
    }')"
fi

max_bytes="$(awk -v mb="$max_mb" 'BEGIN { printf "%d", mb * 1000 * 1000 }')"

start_size_watcher() {
  (
    started_at="$(date +%s)"
    while is_running "$rec_pid"; do
      if [[ -f "$output_path" ]]; then
        bytes="$(wc -c < "$output_path" | tr -d '[:space:]')"
        if [[ "${bytes:-0}" -ge "$max_bytes" ]]; then
          printf 'record_sm_rev: size cap reached; finalizing\n' >&2
          kill -INT "$rec_pid" >/dev/null 2>&1 || true
          exit 0
        fi
      fi
      if [[ "$duration" != "0" ]]; then
        now="$(date +%s)"
        if [[ $((now - started_at)) -ge "$duration" ]]; then
          printf 'record_sm_rev: duration reached; finalizing\n' >&2
          kill -INT "$rec_pid" >/dev/null 2>&1 || true
          exit 0
        fi
      fi
      sleep 1
    done
  ) &
  watcher_pid="$!"
}

case "$backend" in
  wayland-gsr)
    require_tool gpu-screen-recorder
    require_tool slurp

    printf 'record_sm_rev: drag a box around the game window to start Wayland recording\n'
    region="$(slurp -f '%wx%h+%x+%y')" || die "region selection cancelled"
    [[ -n "$region" ]] || die "empty capture region"

    gsr_args=(
      -w "$region"
      -f "$fps"
      -c mp4
      -k h264
      -bm cbr
      -q "$bitrate_k"
      -fm cfr
      -cursor no
      -o "$output_path"
    )
    if [[ "$scale_height" != "0" ]]; then
      gsr_args+=(-s "0x${scale_height}")
    fi

    printf 'record_sm_rev: recording region %s to %s at %sfps, bitrate=%sk, cap=%sMB\n' \
      "$region" "$output_path" "$fps" "$bitrate_k" "$max_mb"
    printf 'record_sm_rev: press Ctrl-C once to stop and save\n'
    recorder_kind="gsr"
    setsid gpu-screen-recorder "${gsr_args[@]}" &
    rec_pid="$!"
    start_size_watcher
    ;;

  x11-ffmpeg)
    [[ -n "${DISPLAY:-}" ]] || die 'DISPLAY is not set; use BACKEND=wayland-gsr on Wayland'
    require_tool ffmpeg

    input_args=()
    if [[ -n "$window_id" ]]; then
      require_tool xdotool
      eval "$(xdotool getwindowgeometry --shell "$window_id")"
      [[ "${WIDTH:-0}" -gt 0 && "${HEIGHT:-0}" -gt 0 ]] || die "could not read window geometry"
      input_args=(-f x11grab -draw_mouse 0 -framerate "$fps" -video_size "${WIDTH}x${HEIGHT}" -window_id "$window_id" -i "$DISPLAY")
    else
      input_args=(-f x11grab -draw_mouse 0 -framerate "$fps" -select_region 1 -i "$DISPLAY")
      printf 'record_sm_rev: drag a box around the game window to start X11 recording\n'
    fi

    vf="fps=${fps},scale=trunc(iw/2)*2:trunc(ih/2)*2"
    if [[ "$scale_height" != "0" ]]; then
      vf="fps=${fps},scale=-2:${scale_height}:flags=neighbor"
    fi

    ffmpeg_args=(
      -hide_banner -loglevel warning -y
      "${input_args[@]}"
      -vf "$vf"
      -an
      -c:v libx264 -preset veryfast -tune animation
      -b:v "${bitrate_k}k"
      -maxrate "${bitrate_k}k"
      -bufsize "$((bitrate_k * 2))k"
      -pix_fmt yuv420p
      -fs "$max_bytes"
      -movflags +faststart
    )
    if [[ "$duration" != "0" ]]; then
      ffmpeg_args+=(-t "$duration")
    fi
    ffmpeg_args+=("$output_path")

    printf 'record_sm_rev: writing %s at %sfps, bitrate=%sk, cap=%sMB\n' \
      "$output_path" "$fps" "$bitrate_k" "$max_mb"
    printf 'record_sm_rev: press Ctrl-C once to stop and finalize\n'
    recorder_kind="ffmpeg"
    ffmpeg_control_fifo="$(mktemp -u "${TMPDIR:-/tmp}/record_sm_rev_ffmpeg.XXXXXX")"
    mkfifo "$ffmpeg_control_fifo"
    exec {ffmpeg_control_fd}<>"$ffmpeg_control_fifo"
    setsid ffmpeg "${ffmpeg_args[@]}" < "$ffmpeg_control_fifo" &
    rec_pid="$!"
    rm -f "$ffmpeg_control_fifo"
    ffmpeg_control_fifo=""
    ;;

  *)
    die "invalid BACKEND=$backend; expected auto, wayland-gsr, or x11-ffmpeg"
    ;;
esac

set +e
finished_pid=""
if [[ -n "$game_pid" ]]; then
  wait -n -p finished_pid "$game_pid" "$rec_pid"
else
  wait -n -p finished_pid "$rec_pid"
fi
exit_status=$?
set -e

if [[ "$finished_pid" == "$game_pid" ]]; then
  game_pid=""
  stop_recorder
else
  rec_pid=""
  if is_running "$watcher_pid"; then
    kill -TERM "$watcher_pid" >/dev/null 2>&1 || true
    wait "$watcher_pid" || true
    watcher_pid=""
  fi
  if is_running "$game_pid"; then
    kill -TERM "$game_pid" >/dev/null 2>&1 || true
    wait "$game_pid" || true
    game_pid=""
  fi
fi

trap - EXIT INT TERM
if [[ -n "$ffmpeg_control_fd" ]]; then
  exec {ffmpeg_control_fd}>&- || true
  ffmpeg_control_fd=""
fi

if [[ -f "$output_path" ]]; then
  bytes="$(wc -c < "$output_path" | tr -d '[:space:]')"
  printf 'record_sm_rev: wrote %s bytes to %s\n' "$bytes" "$output_path"
fi

exit "$exit_status"
