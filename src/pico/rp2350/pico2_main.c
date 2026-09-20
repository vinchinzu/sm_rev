#include "hardware/gpio.h"
#include "hardware/structs/watchdog.h"
#include "hardware/watchdog.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"

#include <stdint.h>
#include <stdio.h>

#include "explorer_buttons.h"
#include "ida_types.h"
#include "mini/mini_game.h"
#include "pico_ls_assets.h"
#include "pico_ls_room.h"
#include "pico_oam_from_samus.h"
#include "pico_oam_gunship.h"
#include "pico_viewport.h"
#include "sm_rtl.h"
#include "st7789_explorer.h"
#include "variables.h"

#define RGB565(r, g, b) \
  ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xF8) >> 3)))

enum {
  kPico2ViewportWidth = 256,
  kPico2ViewportHeight = 224,
  kPico2ReportFrames = 30,
  kExplorerPinA = 12,
  kExplorerPinB = 13,
  kExplorerPinX = 14,
  kExplorerPinY = 15,
  kExplorerBootselMask =
      kExplorerBtnA | kExplorerBtnB | kExplorerBtnX | kExplorerBtnY,
  /* ------------------------------------------------------------------ *
   *  sm_rev-khe, candidate #1 for the reported "screen locks up".        *
   *                                                                      *
   *  The Explorer has four buttons and all four are gameplay: Y=Right,    *
   *  X=Left, B=Down, A=Jump. The BOOTSEL escape chord is all four at      *
   *  once, and it used to fire after ONE second -- about 19 frames at the *
   *  measured 53 ms/frame. A player who jumps while holding left, right   *
   *  and down, or who simply palms the board, hits that chord. When it    *
   *  fires, reset_usb_boot() leaves the panel holding its last frame and  *
   *  takes the CDC device away: glass frozen, log dead, no crash. That is *
   *  indistinguishable from a hang from the outside, and it is exactly    *
   *  the reported shape -- intermittent, only while playing, never idle.  *
   *                                                                      *
   *  Not reproducible from here (the board is reserved), so this does not *
   *  remove the chord, it makes it (a) much harder to hit by accident and *
   *  (b) impossible to hit SILENTLY: the hold is now 3 s and every ~500ms *
   *  of it prints a countdown, so if this is the lockup the CDC log ends  *
   *  with "BOOTSEL chord" instead of ending mid-frame.                    *
   *                                                                      *
   *  Field test for the orchestrator: during a freeze, look at the LED.   *
   *  reset_usb_boot() blinks it; a real hang leaves it steady.            *
   * ------------------------------------------------------------------ */
  kExplorerBootselHoldUs = 3000000,
  kExplorerBootselWarnUs = 500000,
  /* BG maps are 64x32 tiles (512x256 px); the viewport is 256x224, so the
   * camera may travel this far inside the packed window before it runs out. */
  kPico2MaxScrollX = 512 - kPico2ViewportWidth,
  kPico2MaxScrollY = 256 - kPico2ViewportHeight
};

static PicoFramePacket s_pkt;
static uint64_t s_bootsel_held_from;
static unsigned s_bootsel_warned;
static int s_last_frame_index = -1;

/* ======================================================================== *
 *  LOCKUP FORENSICS  --  sm_rev-khe                                        *
 *                                                                          *
 *  The reported symptom is a freeze after a while of play: intermittent and *
 *  input-dependent (an idle board survives indefinitely). Nothing in this   *
 *  tree reproduces it on the host, so rather than guess at a fix this adds  *
 *  the machinery that makes the NEXT lockup name itself over CDC.          *
 *                                                                          *
 *  Three parts:                                                            *
 *                                                                          *
 *  1. A hardware watchdog, kicked once per main-loop iteration. Any hang    *
 *     anywhere -- an unbounded wait, a spin in the sim, a fault handler --  *
 *     becomes a reboot after kWatchdogMs instead of a dead console.         *
 *                                                                          *
 *  2. Breadcrumbs in watchdog scratch[0..3], which survive a watchdog       *
 *     reset. They are rewritten at each phase boundary, so the reboot print *
 *     says WHERE it died and in what game state. Layout:                    *
 *       scratch[0] = kCrumbMagic | phase   (phase kCrumb* below)            *
 *       scratch[1] = frame counter                                          *
 *       scratch[2] = pose<<24 | anim<<16 | frame_index   (or fault PC)      *
 *       scratch[3] = scanline<<16 | joy                  (or fault LR)      *
 *     The SDK owns scratch[4..7] (watchdog.c reboot magic); we stay off it. *
 *                                                                          *
 *  3. An isr_hardfault override. The SDK default is an infinite loop, which *
 *     looks EXACTLY like the reported symptom: glass frozen, log silent, no *
 *     clue. Ours records the stacked PC/LR and reboots, so a fault turns    *
 *     into a line of text on the next boot.                                 *
 *                                                                          *
 *  Also a painted-stack high-water mark. The default stack is 2 KiB and the *
 *  linker map puts __StackBottom at 0x20081800 with nothing but unused      *
 *  SCRATCH_X/Y below it, so an overflow has ~6 KiB of dead space to fall    *
 *  through before it could reach anything live -- but "probably fine" is    *
 *  not a measurement, so the stats line now carries the real number.        *
 * ======================================================================== */
enum {
  kCrumbMagic = 0x5D4B0000u,
  kCrumbBoot = 0x01,
  kCrumbPoll = 0x02,
  kCrumbStep = 0x03,
  kCrumbPack = 0x04,
  kCrumbBegin = 0x05,
  kCrumbRaster = 0x06,
  kCrumbLine = 0x07,
  kCrumbEnd = 0x08,
  kCrumbPrint = 0x09,
  kCrumbBootsel = 0x0A,
  kCrumbFault = 0xF0,
  /* A healthy frame is ~53 ms. 4 s is ~75 frames of slack: far beyond any
   * legitimate stall (a 20 ms SPI timeout, a 0.5 s blocked printf) and still
   * short enough that a lockup reboots while the player is still watching. */
  kWatchdogMs = 4000
};

static void crumb(uint32_t phase) {
  watchdog_hw->scratch[0] = kCrumbMagic | (phase & 0xFFu);
}

static void crumb_bootsel(void) {
  watchdog_hw->scratch[0] = kCrumbMagic | kCrumbBootsel;
}

static void crumb_state(uint32_t frame, int frame_index, uint16_t joy) {
  watchdog_hw->scratch[1] = frame;
  watchdog_hw->scratch[2] = ((uint32_t)(samus_pose & 0xFF) << 24) |
                            ((uint32_t)(samus_anim_frame & 0xFF) << 16) |
                            ((uint32_t)frame_index & 0xFFFFu);
  watchdog_hw->scratch[3] = (uint32_t)joy;
}

static const char *crumb_phase_name(uint32_t phase) {
  switch (phase & 0xFFu) {
    case kCrumbBoot: return "boot";
    case kCrumbPoll: return "buttons";
    case kCrumbStep: return "MiniStepButtons";
    case kCrumbPack: return "pack_from_samus";
    case kCrumbBegin: return "BeginFrame/letterbox";
    case kCrumbRaster: return "raster";
    case kCrumbLine: return "WriteRgb565Line";
    case kCrumbEnd: return "EndFrame/letterbox";
    case kCrumbPrint: return "printf";
    case kCrumbBootsel: return "BOOTSEL chord (reset_usb_boot, not a crash)";
    case kCrumbFault: return "HARD FAULT";
    default: return "?";
  }
}

/*
 * Runs with the fault's exception frame at *frame (r0 r1 r2 r3 r12 lr pc xpsr).
 * No printf here: the USB mutex may be held by the code we faulted out of.
 * Record and reboot; main() prints it on the way back up.
 */
void __attribute__((used)) sm_khe_fault_handler(const uint32_t *frame);
void __attribute__((used)) sm_khe_fault_handler(const uint32_t *frame) {
  watchdog_hw->scratch[0] = kCrumbMagic | kCrumbFault;
  watchdog_hw->scratch[2] = frame[6]; /* stacked PC */
  watchdog_hw->scratch[3] = frame[5]; /* stacked LR */
  watchdog_reboot(0, 0, 0);
  for (;;)
    tight_loop_contents();
}

void __attribute__((naked)) isr_hardfault(void);
void __attribute__((naked)) isr_hardfault(void) {
  __asm volatile("mov r0, sp\n"
                 "b   sm_khe_fault_handler\n");
}

/* Painted-stack high-water mark. __StackBottom/__StackTop come from the SDK
 * linker script (memmap_default.ld). */
extern uint8_t __StackBottom[];
extern uint8_t __StackTop[];

enum { kStackPaint = 0xC5 };

/* Addresses, not pointers into an object: the paint deliberately runs over
 * stack space no C object owns, so keep the arithmetic away from anything the
 * compiler could bounds-reason about. */
/* noinline matters: `here` must land in THIS function's frame, below main's,
 * or the paint would run over main's own locals. */
static void __attribute__((noinline)) stack_paint(void) {
  volatile uint8_t here = 0;
  uintptr_t lo = (uintptr_t)__StackBottom;
  uintptr_t hi = (uintptr_t)&here - 64u; /* leave this frame plus margin */

  while (lo < hi) {
    *(volatile uint8_t *)lo = (uint8_t)kStackPaint;
    lo++;
  }
}

static uint32_t stack_used_max(void) {
  uintptr_t p = (uintptr_t)__StackBottom;
  uintptr_t top = (uintptr_t)__StackTop;

  while (p < top && *(volatile uint8_t *)p == (uint8_t)kStackPaint)
    p++;
  return (uint32_t)(top - p);
}

static void report_boot_reason(void) {
  uint32_t crumb0 = watchdog_hw->scratch[0];
  uint32_t reason = watchdog_hw->reason;

  if ((crumb0 & 0xFFFF0000u) != kCrumbMagic) {
    printf("pico2 boot: cold (wdog reason=%u, no breadcrumb)\n",
           (unsigned)reason);
  } else if ((crumb0 & 0xFFu) == kCrumbFault) {
    printf("pico2 boot: *** HARD FAULT *** pc=%08x lr=%08x frame=%u "
           "(wdog reason=%u)\n",
           (unsigned)watchdog_hw->scratch[2], (unsigned)watchdog_hw->scratch[3],
           (unsigned)watchdog_hw->scratch[1], (unsigned)reason);
  } else {
    uint32_t st = watchdog_hw->scratch[2];
    printf("pico2 boot: *** WATCHDOG *** hung in %s frame=%u line=%u pose=%u "
           "af=%u fi=%u joy=%04x (wdog reason=%u)\n",
           crumb_phase_name(crumb0), (unsigned)watchdog_hw->scratch[1],
           (unsigned)(watchdog_hw->scratch[3] >> 16),
           (unsigned)((st >> 24) & 0xFF), (unsigned)((st >> 16) & 0xFF),
           (unsigned)(st & 0xFFFFu),
           (unsigned)(watchdog_hw->scratch[3] & 0xFFFFu), (unsigned)reason);
  }
  watchdog_hw->scratch[0] = 0;
  crumb(kCrumbBoot);
}

static void enter_bootloader(void) {
  /* Breadcrumb first: this leaves via reset_usb_boot(), so if the board comes
   * back with a fresh image the last thing it did is still on record. */
  crumb_bootsel();
  printf("pico2: A+B+X+Y held %ums, reset_usb_boot -- NOT a crash\n",
         (unsigned)(kExplorerBootselHoldUs / 1000u));
  stdio_flush();
  sleep_ms(50);
  reset_usb_boot(1u << PICO_DEFAULT_LED_PIN, 0);
}

static void maybe_enter_bootloader(unsigned pressed) {
  uint64_t held;

  if ((pressed & kExplorerBootselMask) != kExplorerBootselMask) {
    if (s_bootsel_held_from != 0)
      printf("pico2: BOOTSEL chord released\n");
    s_bootsel_held_from = 0;
    s_bootsel_warned = 0;
    return;
  }
  if (s_bootsel_held_from == 0) {
    s_bootsel_held_from = time_us_64();
    s_bootsel_warned = 0;
    printf("pico2: BOOTSEL chord (A+B+X+Y) down -- hold %ums to reboot\n",
           (unsigned)(kExplorerBootselHoldUs / 1000u));
    return;
  }
  held = time_us_64() - s_bootsel_held_from;
  if (held >= (uint64_t)kExplorerBootselHoldUs) {
    enter_bootloader();
    return;
  }
  /* Loud on the way down: an accidental chord must be visible in the log even
   * if the board vanishes before the final line makes it out. */
  while ((uint64_t)s_bootsel_warned * kExplorerBootselWarnUs <= held) {
    s_bootsel_warned++;
    printf("pico2: BOOTSEL chord held %ums/%ums\n",
           (unsigned)(held / 1000u),
           (unsigned)(kExplorerBootselHoldUs / 1000u));
  }
}

static void explorer_buttons_init(void) {
  const unsigned pins[4] = {kExplorerPinA, kExplorerPinB, kExplorerPinX,
                            kExplorerPinY};
  unsigned i;
  for (i = 0; i < 4; i++) {
    gpio_init(pins[i]);
    gpio_set_dir(pins[i], GPIO_IN);
    gpio_pull_up(pins[i]);
  }
}

static unsigned explorer_buttons_poll(void) {
  unsigned pressed = 0;
  if (!gpio_get(kExplorerPinA))
    pressed |= kExplorerBtnA;
  if (!gpio_get(kExplorerPinB))
    pressed |= kExplorerBtnB;
  if (!gpio_get(kExplorerPinX))
    pressed |= kExplorerBtnX;
  if (!gpio_get(kExplorerPinY))
    pressed |= kExplorerBtnY;
  return pressed;
}

static void pack_from_samus(PicoFramePacket *pkt, uint32_t frame_id,
                            uint16_t joy) {
  /* PicoLsRoom_Install put the real Landing Site collision in mini, so these
   * are already room world coordinates. Nothing to re-base. */
  int world_x = (int)samus_x_pos;
  int world_y = (int)samus_y_pos;
  int pose = (int)samus_pose;
  /* The sim's own counter. PicoOam_InstallSamusBank91() gives the vanilla
   * clock (Samus_Animate -> Samus_HandleAnimDelay) the bank 0x91 delay bytes
   * it needs, so this advances on its own; there is no wall-clock stand-in. */
  int anim = (int)samus_anim_frame;
  int frame_index = PicoOam_SamusFrameIndex(pose, anim);
  int cam_x = world_x - kPico2ViewportWidth / 2;
  int cam_y = world_y - kPico2ViewportHeight / 2;
  int sx;
  int sy;

  /* Follow Samus, but never past the edge of the packed window. */
  if (cam_x < (int)kPicoLsExtractCameraX)
    cam_x = (int)kPicoLsExtractCameraX;
  if (cam_x > (int)kPicoLsExtractCameraX + kPico2MaxScrollX)
    cam_x = (int)kPicoLsExtractCameraX + kPico2MaxScrollX;
  if (cam_y < (int)kPicoLsExtractCameraY)
    cam_y = (int)kPicoLsExtractCameraY;
  if (cam_y > (int)kPicoLsExtractCameraY + kPico2MaxScrollY)
    cam_y = (int)kPicoLsExtractCameraY + kPico2MaxScrollY;

  sx = world_x - cam_x;
  sy = world_y - cam_y - PicoOam_SamusYOffset(pose);

  /* Vanilla re-DMAs Samus CHR whenever the frame changes; so do we. */
  if (frame_index != s_last_frame_index) {
    if (PicoOam_PlantSamusFrame(pkt, frame_index))
      s_last_frame_index = frame_index;
    else
      printf("pico2: PlantSamusFrame(%d) failed\n", frame_index);
  }

  pkt->frame_id = frame_id;
  pkt->vsync_token = (uint16_t)kPicoFramePacketVsync;
  pkt->joypad_echo = joy;
  pkt->bg1hofs = (uint16_t)(cam_x - (int)kPicoLsExtractCameraX);
  pkt->bg1vofs = (uint16_t)(cam_y - (int)kPicoLsExtractCameraY);
  /* room_91F8.json scroll.bgScrolling = 0x0181 (sm_rev-k5q.5).
   * layer2_scroll_x 0x81 makes vanilla CalculateLayer2Xpos() compute
   * layer1_x_pos / 2, so BG2 parallaxes at HALF the BG1 rate. layer2_scroll_y
   * 0x01 makes CalculateLayer2Ypos() bail out before reg_BG2VOFS is touched,
   * so BG2 never scrolls vertically: it holds its room-load value, and the
   * packed map was expanded from exactly that row. */
  pkt->bg2hofs = (uint16_t)(pkt->bg1hofs >> 1);
  pkt->bg2vofs = (uint16_t)kPicoLsBg2VerticalScroll;

  PicoOam_WriteSamusFrame(pkt, frame_index, sx, sy);
  /* Fixed extract-space OAM, rewritten after the Samus slots are cleared. */
  PicoOam_WriteGunship(pkt, -(int)pkt->bg1hofs, -(int)pkt->bg1vofs);
}

static void present_frame(const PicoFramePacket *pkt, uint32_t *raster_us,
                          uint32_t *spi_us) {
  PicoPpuState ppu;
  /* static: ~1KB of line buffers would not fit the default 2KiB stack. */
  static uint16_t line256[kPicoScreenWidth];
  static uint16_t line240[kPicoPanelWidth];
  uint64_t t;
  int y;

  PicoFramePacket_ToPpu(pkt, &ppu);
  *raster_us = 0;
  *spi_us = 0;

  t = time_us_64();
  crumb(kCrumbBegin);
  St7789Explorer_BeginFrame();
  for (y = 0; y < kPicoViewportLetterboxY; y++)
    St7789Explorer_WriteSolidLine(0, kPicoPanelWidth);
  *spi_us += (uint32_t)(time_us_64() - t);

  for (y = 0; y < kPicoScreenHeight; y++) {
    /* Scanline in the high half so a watchdog reboot names the exact line. */
    watchdog_hw->scratch[3] =
        (watchdog_hw->scratch[3] & 0xFFFFu) | ((uint32_t)y << 16);
    crumb(kCrumbRaster);
    t = time_us_64();
    /* sm_rev-k5q.7 item 6: the panel only ever shows columns 8..247, and
     * PicoViewport_CropLineRgb565 throws the other 16 away. Raster the 240
     * that survive. The columns outside the range keep whatever line256 held;
     * nothing reads them. Host renders still call PicoScanline_Mode1 and stay
     * 256 wide and byte-identical. */
    PicoScanline_Mode1Range(&ppu, y, line256, kPicoViewportCropX,
                            kPicoViewportCropX + kPicoPanelWidth);
    PicoViewport_CropLineRgb565(line256, line240);
    *raster_us += (uint32_t)(time_us_64() - t);

    crumb(kCrumbLine);
    t = time_us_64();
    St7789Explorer_WriteRgb565Line(line240, kPicoPanelWidth);
    *spi_us += (uint32_t)(time_us_64() - t);
  }

  t = time_us_64();
  crumb(kCrumbEnd);
  for (y = 0; y < kPicoViewportLetterboxY; y++)
    St7789Explorer_WriteSolidLine(0, kPicoPanelWidth);
  St7789Explorer_EndFrame();
  *spi_us += (uint32_t)(time_us_64() - t);
}

int main(void) {
  MiniGameState *state;
  uint32_t frame = 0;
  uint32_t acc_step = 0;
  uint32_t acc_pack = 0;
  uint32_t acc_raster = 0;
  uint32_t acc_spi = 0;
  uint32_t acc_n = 0;

  stack_paint();
  stdio_init_all();
  /* Before anything else touches the watchdog registers: scratch[0..3] carry
   * the previous run's last breadcrumb across a watchdog reset. */
  report_boot_reason();

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
  gpio_put(PICO_DEFAULT_LED_PIN, 1);

  explorer_buttons_init();
  St7789Explorer_Init();
  /* Black, not magenta: a boot hang must not look like the old ship-1 fill. */
  St7789Explorer_Fill(RGB565(0, 0, 0));

  /* Before MiniCreate: with no filesystem on-device, this is the only way
   * mini gets Landing Site collision instead of its flat fallback room. */
  if (!PicoLsRoom_Install())
    printf("pico2: PicoLsRoom_Install failed\n");
  /* Also before MiniCreate: mini seeds the animation clock while initialising
   * Samus, and that read needs bank 0x91 already in place. */
  if (!PicoOam_InstallSamusBank91())
    printf("pico2: PicoOam_InstallSamusBank91 failed\n");

  state = MiniCreate(kPico2ViewportWidth, kPico2ViewportHeight);
  if (state == NULL) {
    printf("pico2: MiniCreate failed\n");
    for (;;) {
      gpio_xor_mask(1u << PICO_DEFAULT_LED_PIN);
      sleep_ms(100);
    }
  }

  PicoFramePacket_InitLandingSiteExtracted(&s_pkt);
  PicoOam_ParkAllSprites(&s_pkt);
  if (!PicoOam_PlantGunshipGfx(&s_pkt))
    printf("pico2: PlantGunshipGfx failed\n");
  pack_from_samus(&s_pkt, 1, 0);

  printf("pico2 ls+explorer map Y=Right X=Left B=Down A=Jump\n");
  {
    MiniRoomInfo info;
    MiniStubs_GetRoomInfo(&info);
    printf("pico2 room %s src=%s %dx%d blocks, spawn %u,%u at %u,%u\n",
           info.room_handle, MiniStubs_RoomSourceName(info.room_source),
           info.room_width_blocks, info.room_height_blocks,
           (unsigned)kPicoLsRoomSpawnX, (unsigned)kPicoLsRoomSpawnY,
           (unsigned)samus_x_pos, (unsigned)samus_y_pos);
  }
  printf("pico2 samus frames=%d packed, stand=%d run=%d turn=%d\n",
         PicoOam_SamusFrameTotal(), PicoOam_SamusPoseFrames(1),
         PicoOam_SamusPoseFrames(9), PicoOam_SamusPoseFrames(0x25));
  printf("pico2 samus bank91=%d pose=%u af=%u timer=%u yoff=%d yrad=%d\n",
         g_samus_bank91 != NULL, (unsigned)samus_pose,
         (unsigned)samus_anim_frame, (unsigned)samus_anim_frame_timer,
         PicoOam_SamusYOffset((int)samus_pose),
         PicoOam_SamusYRadius((int)samus_pose));
  printf("pico2 BOOTSEL: picotool -f, or hold A+B+X+Y %us\n",
         (unsigned)(kExplorerBootselHoldUs / 1000000u));
  printf("pico2 stack=%u bytes watchdog=%ums\n",
         (unsigned)(__StackTop - __StackBottom), (unsigned)kWatchdogMs);

  /* Last thing before the loop: everything above this line is allowed to take
   * as long as it likes (SWRESET sleeps 150 ms, MiniCreate allocates). */
  watchdog_enable(kWatchdogMs, 1);

  for (;;) {
    unsigned pressed;

    watchdog_update();
    crumb(kCrumbPoll);
    pressed = explorer_buttons_poll();
    maybe_enter_bootloader(pressed);
    uint16_t joy = ExplorerButtons_ToJoypad(pressed);
    uint64_t t;
    uint32_t step_us;
    uint32_t pack_us;
    uint32_t raster_us;
    uint32_t spi_us;

    crumb_state(frame, s_last_frame_index, joy);
    crumb(kCrumbStep);
    t = time_us_64();
    MiniStepButtons(state, joy, false);
    step_us = (uint32_t)(time_us_64() - t);

    crumb(kCrumbPack);
    t = time_us_64();
    pack_from_samus(&s_pkt, frame + 2, joy);
    pack_us = (uint32_t)(time_us_64() - t);
    /* Refresh now that the sim has moved and the frame index is resolved. */
    crumb_state(frame, s_last_frame_index, joy);

    present_frame(&s_pkt, &raster_us, &spi_us);

    acc_step += step_us;
    acc_pack += pack_us;
    acc_raster += raster_us;
    acc_spi += spi_us;
    acc_n++;
    frame++;

    gpio_put(PICO_DEFAULT_LED_PIN, pressed ? 1 : (int)((frame / kPico2ReportFrames) & 1));

    if (acc_n >= kPico2ReportFrames) {
      uint32_t present_us = (acc_raster + acc_spi) / acc_n;
      uint32_t frame_us =
          (acc_step + acc_pack + acc_raster + acc_spi) / acc_n;
      uint32_t stall_dma;
      uint32_t stall_abort;
      uint32_t stall_spi;

      St7789Explorer_GetStalls(&stall_dma, &stall_abort, &stall_spi);
      crumb(kCrumbPrint);
      printf("pico2 step=%u pack=%u raster=%u spi=%u present=%u frame=%u us "
             "x=%u y=%u pose=%u af=%u joy=%04x btn=%x stall=%u/%u/%u stk=%u\n",
             acc_step / acc_n, acc_pack / acc_n, acc_raster / acc_n,
             acc_spi / acc_n, present_us, frame_us, (unsigned)samus_x_pos,
             (unsigned)samus_y_pos, (unsigned)samus_pose,
             (unsigned)samus_anim_frame, (unsigned)joy, pressed,
             (unsigned)stall_dma, (unsigned)stall_abort, (unsigned)stall_spi,
             (unsigned)stack_used_max());
      acc_step = acc_pack = acc_raster = acc_spi = acc_n = 0;
    }
  }
}
