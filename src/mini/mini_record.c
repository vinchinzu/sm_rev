#include "mini_record.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <SDL.h>

#if defined(_WIN32)
#include <io.h>
#define access _access
#define F_OK 0
#define popen _popen
#define pclose _pclose
#define mkdir(path, mode) _mkdir(path)
#else
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "mini_defs.h"

enum {
  kMiniRecordInputFps = 60,
  kMiniRecordOutputFps = 30,
  kMiniRecordSizeLimitMegabytes = 10,
};

static bool MiniRecord_HasOutput(const MiniRecorder *recorder) {
  return recorder->output_path[0] != '\0' && access(recorder->output_path, F_OK) == 0;
}

static bool MiniRecord_EnsureDir(const char *path) {
  if (mkdir(path, 0777) == 0)
    return true;
  return errno == EEXIST;
}

static void MiniRecord_BuildPath(char *dst, size_t dst_size) {
  time_t now = time(NULL);
  struct tm tm_now;
  char *base_path = SDL_GetBasePath();
  const char *prefix = (base_path != NULL && base_path[0] != '\0') ? base_path : ".";
#if defined(_WIN32)
  localtime_s(&tm_now, &now);
#else
  localtime_r(&now, &tm_now);
#endif
  snprintf(dst, dst_size, "%s/out", prefix);
  if (!MiniRecord_EnsureDir(dst)) {
    snprintf(dst, dst_size, "out");
    MiniRecord_EnsureDir(dst);
  }
  size_t len = strlen(dst);
  strftime(dst + len, dst_size - len, "/mini_recording_%Y%m%d_%H%M%S.mp4", &tm_now);
  SDL_free(base_path);
}

bool MiniRecord_Start(MiniRecorder *recorder) {
  char command[PATH_MAX + 256];

  memset(recorder, 0, sizeof(*recorder));
  MiniRecord_BuildPath(recorder->output_path, sizeof(recorder->output_path));

#if !defined(_WIN32)
  signal(SIGPIPE, SIG_IGN);
#endif

  snprintf(command, sizeof(command),
           "ffmpeg -loglevel error -y "
           "-f rawvideo -pixel_format bgra -video_size %dx%d -framerate %d -i - "
           "-vf fps=%d "
           "-an -c:v libx264 -preset veryfast -crf 34 -pix_fmt yuv420p -movflags +faststart "
           "-fs %dM %s",
           kMiniGameWidth, kMiniGameHeight, kMiniRecordInputFps,
           kMiniRecordOutputFps,
           kMiniRecordSizeLimitMegabytes, recorder->output_path);

  recorder->pipe = popen(command, "w");
  if (recorder->pipe == NULL) {
    fprintf(stderr, "mini: failed to start ffmpeg recorder: %s\n", strerror(errno));
    recorder->output_path[0] = '\0';
    return false;
  }

  setvbuf(recorder->pipe, NULL, _IONBF, 0);
  fprintf(stderr, "mini: recording quick clip to %s\n", recorder->output_path);
  return true;
}

bool MiniRecord_WriteFrame(MiniRecorder *recorder, const uint32_t *pixels) {
  const uint8_t *src = (const uint8_t *)pixels;
  size_t frame_bytes = (size_t)kMiniGameWidth * kMiniGameHeight * sizeof(uint32_t);
  size_t written = 0;

  if (recorder->pipe == NULL || recorder->closed_early)
    return true;

  while (written < frame_bytes) {
    size_t chunk = fwrite(src + written, 1, frame_bytes - written, recorder->pipe);
    written += chunk;
    if (written == frame_bytes) {
      recorder->frames_written++;
      return true;
    }
    if (ferror(recorder->pipe))
      break;
  }

  clearerr(recorder->pipe);
  recorder->closed_early = true;
  fprintf(stderr, "mini: recording stopped after reaching the %d MB cap\n",
          kMiniRecordSizeLimitMegabytes);
  return true;
}

bool MiniRecord_Finish(MiniRecorder *recorder) {
  int status;

  if (recorder->pipe == NULL)
    return true;

  status = pclose(recorder->pipe);
  recorder->pipe = NULL;
  if (recorder->frames_written > 0 && MiniRecord_HasOutput(recorder))
    fprintf(stderr, "mini: saved %d recorded frames to %s\n",
            recorder->frames_written, recorder->output_path);
  if (status == 0)
    return true;

  if (recorder->closed_early && MiniRecord_HasOutput(recorder))
    return true;

  fprintf(stderr, "mini: ffmpeg did not finish recording cleanly\n");
  return false;
}
