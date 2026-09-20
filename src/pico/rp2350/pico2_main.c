#include "hardware/gpio.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"

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
  kExplorerBootselHoldUs = 1000000,
  /* BG maps are 64x32 tiles (512x256 px); the viewport is 256x224, so the
   * camera may travel this far inside the packed window before it runs out. */
  kPico2MaxScrollX = 512 - kPico2ViewportWidth,
  kPico2MaxScrollY = 256 - kPico2ViewportHeight
};

static PicoFramePacket s_pkt;
static uint64_t s_bootsel_held_from;
static int s_last_frame_index = -1;
/* sm_rev-28r: owns the boot/chord gate and the B morph-toggle latch. */
static ExplorerButtonsState s_buttons;

static void enter_bootloader(void) {
  printf("pico2: A+B+X+Y held, reset_usb_boot\n");
  stdio_flush();
  sleep_ms(50);
  reset_usb_boot(1u << PICO_DEFAULT_LED_PIN, 0);
}

static void maybe_enter_bootloader(unsigned pressed) {
  if ((pressed & kExplorerBootselMask) != kExplorerBootselMask) {
    s_bootsel_held_from = 0;
    return;
  }
  if (s_bootsel_held_from == 0) {
    s_bootsel_held_from = time_us_64();
    return;
  }
  if (time_us_64() - s_bootsel_held_from >= (uint64_t)kExplorerBootselHoldUs)
    enter_bootloader();
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
  St7789Explorer_BeginFrame();
  for (y = 0; y < kPicoViewportLetterboxY; y++)
    St7789Explorer_WriteSolidLine(0, kPicoPanelWidth);
  *spi_us += (uint32_t)(time_us_64() - t);

  for (y = 0; y < kPicoScreenHeight; y++) {
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

    t = time_us_64();
    St7789Explorer_WriteRgb565Line(line240, kPicoPanelWidth);
    *spi_us += (uint32_t)(time_us_64() - t);
  }

  t = time_us_64();
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

  stdio_init_all();

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
  gpio_put(PICO_DEFAULT_LED_PIN, 1);

  explorer_buttons_init();
  ExplorerButtons_Init(&s_buttons);
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

  /* sm_rev-28r: B is a morph TOGGLE, not a plain Down. The Pico runs
   * MiniAuthoredMovement_Step (g_rom == NULL), where one new Down morphs
   * instantly and only a new Up unmorphs -- and this board has no Up button.
   * ExplorerButtons_Step() emits Down when she is standing and Up when she is
   * a ball, and swallows anything already held at boot. */
  printf("pico2 ls+explorer map Y=Right X=Left A=Jump B=Morph/Unmorph\n");
  printf("pico2 input: buttons ignored until all four are released once\n");
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
  printf("pico2 BOOTSEL: picotool -f, or hold A+B+X+Y 1s\n");

  for (;;) {
    unsigned pressed = explorer_buttons_poll();
    maybe_enter_bootloader(pressed);
    uint16_t joy =
        ExplorerButtons_Step(&s_buttons, pressed, (unsigned)samus_movement_type);
    uint64_t t;
    uint32_t step_us;
    uint32_t pack_us;
    uint32_t raster_us;
    uint32_t spi_us;

    t = time_us_64();
    MiniStepButtons(state, joy, false);
    step_us = (uint32_t)(time_us_64() - t);

    t = time_us_64();
    pack_from_samus(&s_pkt, frame + 2, joy);
    pack_us = (uint32_t)(time_us_64() - t);

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
      printf("pico2 step=%u pack=%u raster=%u spi=%u present=%u frame=%u us "
             "x=%u y=%u pose=%u mvt=%u af=%u joy=%04x btn=%x\n",
             acc_step / acc_n, acc_pack / acc_n, acc_raster / acc_n,
             acc_spi / acc_n, present_us, frame_us, (unsigned)samus_x_pos,
             (unsigned)samus_y_pos, (unsigned)samus_pose,
             (unsigned)samus_movement_type, (unsigned)samus_anim_frame,
             (unsigned)joy, pressed);
      acc_step = acc_pack = acc_raster = acc_spi = acc_n = 0;
    }
  }
}
