#include "st7789_explorer.h"

#include <stdio.h>
#include <string.h>

#include "hardware/dma.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

/* Seeded from pico2_explorer_bringup/st7789_fill.c (proven 2026-09-19). */

/* ======================================================================== *
 *  REVERTIBLE SPI OVERCLOCK  --  sm_rev-k5q.7                              *
 *                                                                          *
 *  This is the ONE change in this bead that depends on what the glass and  *
 *  the wiring tolerate rather than on logic. To back it out, and nothing   *
 *  else, set this back to 32000000. Everything else in this file (16-bit   *
 *  frames, DMA, double buffering) is clock-independent and stays.          *
 *                                                                          *
 *  What the number actually buys, on RP2350 with clk_peri = 150 MHz:       *
 *  the PL022 divides 150 MHz by (CPSDVSR * (1 + SCR)) with CPSDVSR even,   *
 *  and spi_set_baudrate() rounds DOWN to the next reachable rung. The      *
 *  reachable rungs near here are 75.0 / 37.5 / 25.0 / 18.75 MHz, so:       *
 *                                                                          *
 *      request 32000000  -> 25.0 MHz   (what shipped before this bead:     *
 *                                       the old "32 MHz" never existed)    *
 *      request 62500000  -> 37.5 MHz   (this build)                        *
 *      request 75000000  -> 75.0 MHz   (next rung; untested on this panel) *
 *                                                                          *
 *  Init prints the rate spi_init() actually achieved, so the boot log is   *
 *  the authority, not this comment.                                        *
 * ======================================================================== */
#ifndef ST7789_EXPLORER_SPI_HZ
#define ST7789_EXPLORER_SPI_HZ 62500000
#endif

enum {
  kSpiHz = ST7789_EXPLORER_SPI_HZ
};

#define PIN_SCK 18
#define PIN_MOSI 19
#define PIN_CS 17
#define PIN_DC 16
#define PIN_BL 20

#define ST7789_SWRESET 0x01
#define ST7789_SLPOUT 0x11
#define ST7789_INVON 0x21
#define ST7789_DISPON 0x29
#define ST7789_CASET 0x2A
#define ST7789_RASET 0x2B
#define ST7789_RAMWR 0x2C
#define ST7789_TEON 0x35
#define ST7789_MADCTL 0x36
#define ST7789_COLMOD 0x3A
#define ST7789_PORCTRL 0xB2
#define ST7789_GCTRL 0xB7
#define ST7789_VCOMS 0xBB
#define ST7789_LCMCTRL 0xC0
#define ST7789_VDVVRHEN 0xC2
#define ST7789_VRHS 0xC3
#define ST7789_VDVS 0xC4
#define ST7789_FRCTRL2 0xC6
#define ST7789_PWCTRL1 0xD0
#define ST7789_PVGAMCTRL 0xE0
#define ST7789_NVGAMCTRL 0xE1
#define ST7789_STE 0x44

#define MADCTL_COL_ORDER 0x40
#define MADCTL_SWAP_XY 0x20
#define MADCTL_SCAN_ORDER 0x10
#define MADCTL_RGB 0x08

/*
 * sm_rev-k5q.7, SPI half.
 *
 * Before: every line was byte-swapped into a scratch buffer by a 240-iteration
 * loop and then pushed out with a blocking spi_write_blocking(), so the CPU sat
 * in a polling loop for the whole 480-byte transfer while the raster waited.
 *
 * Now:
 *  - Pixel payload goes out as 16-bit SPI frames. The PL022 shifts a frame
 *    MSB first, so a uint16_t RGB565 lands on the wire high byte first, which
 *    is exactly what the byte-swap loop used to produce. The loop is gone, and
 *    half as many inter-frame gaps are clocked out. Commands stay 8-bit.
 *  - Transfers are DMA'd, so WriteRgb565Line() returns as soon as the DMA is
 *    armed and the caller can raster the next scanline underneath it.
 *  - Two line buffers, so the memcpy for line n+1 runs while line n is on the
 *    wire. Only the DMA start has to wait, and only if the wire is slower than
 *    the raster.
 *
 * Consequence for the instrumentation line in pico2_main.c: the spi= field now
 * measures CPU time in the SPI path (copy, arm, and any stall), not wire time.
 * When the raster is slower than the wire -- which it is -- the transfer is
 * hidden and spi= collapses. present= therefore becomes ~max(raster, wire)
 * instead of the old sum, on one core.
 */
static int s_dma_ch = -1;
static int s_dma_busy;
static uint16_t s_line[2][kSt7789ExplorerWidth];
static int s_line_idx;
static uint16_t s_solid_word;
static uint s_spi_hz;
static uint32_t s_stall_dma;
static uint32_t s_stall_abort;
static uint32_t s_stall_spi;

/* ======================================================================== *
 *  BOUNDED WAITS  --  sm_rev-khe                                           *
 *                                                                          *
 *  Every wait in this file used to be an unbounded `while (busy)`:          *
 *  dma_channel_wait_for_finish_blocking(), spi_is_busy(), and the FIFO      *
 *  poll inside spi_write_blocking(). If the PL022 ever stops draining its   *
 *  TX FIFO -- a stuck DREQ, a format switch that lost SSE, a channel that   *
 *  was aborted underneath us -- the core spins there forever with           *
 *  interrupts on but the main loop dead: the panel holds its last frame and *
 *  the CDC log stops, which is exactly the reported symptom.                *
 *                                                                          *
 *  We cannot prove that is what happens (no board here), so instead of      *
 *  guessing a fix these waits now time out, count the stall, try to recover *
 *  and let the caller carry on. A stalled frame is a torn frame; a stalled  *
 *  frame that never returns is a dead console. St7789Explorer_GetStalls()  *
 *  hands the counts to the main loop, which prints them -- so if this IS    *
 *  the lockup, the next one announces itself over CDC instead of being      *
 *  silent.                                                                  *
 *                                                                          *
 *  Budget: one 240-frame line at the slowest reachable rung (18.75 MHz) is  *
 *  about 205 us, and a whole 240-line frame about 50 ms. 20 ms per single   *
 *  wait is ~100x the expected line time and still well inside the ~53 ms    *
 *  frame budget, so a healthy frame can never hit it.                       *
 * ======================================================================== */
enum {
  kStWaitTimeoutUs = 20000
};

static int st_timed_out(uint64_t t0) {
  return time_us_64() - t0 > (uint64_t)kStWaitTimeoutUs;
}

static void st_spi_idle(void) {
  uint64_t t0 = time_us_64();
  while (spi_is_busy(spi0)) {
    if (st_timed_out(t0)) {
      s_stall_spi++;
      return;
    }
    tight_loop_contents();
  }
}

/*
 * Own 8-bit write, because spi_write_blocking()'s FIFO poll is unbounded.
 * Same semantics otherwise: push every byte, ignore RX, then settle the wire
 * and clear the overrun the TX-only traffic left behind.
 */
static void st_write(const uint8_t *data, size_t n) {
  size_t i;
  uint64_t t0 = time_us_64();

  for (i = 0; i < n; i++) {
    while (!spi_is_writable(spi0)) {
      if (st_timed_out(t0)) {
        s_stall_spi++;
        return;
      }
      tight_loop_contents();
    }
    spi_get_hw(spi0)->dr = (uint32_t)data[i];
  }
  st_spi_idle();
  while (spi_is_readable(spi0))
    (void)spi_get_hw(spi0)->dr;
  spi_get_hw(spi0)->icr = SPI_SSPICR_RORIC_BITS | SPI_SSPICR_RTIC_BITS;
}

/*
 * Bounded stand-in for dma_channel_abort(), whose own `while (dma_hw->abort)`
 * spin is documented as not retiring for a channel parked on a DREQ that never
 * comes -- precisely the case we reach this from. Clear EN first so the channel
 * stops asking for the DREQ, then bound the abort handshake as well.
 */
static void st_dma_abort(void) {
  uint32_t mask = 1u << (uint)s_dma_ch;
  uint64_t t0;

  hw_clear_bits(&dma_hw->ch[(uint)s_dma_ch].al1_ctrl, DMA_CH0_CTRL_TRIG_EN_BITS);
  dma_hw->abort = mask;
  t0 = time_us_64();
  while (dma_hw->abort & mask) {
    if (st_timed_out(t0)) {
      s_stall_abort++;
      break;
    }
    tight_loop_contents();
  }
}

static void st_dma_wait(void) {
  uint64_t t0;

  if (!s_dma_busy)
    return;
  t0 = time_us_64();
  while (dma_channel_is_busy((uint)s_dma_ch)) {
    if (st_timed_out(t0)) {
      s_stall_dma++;
      st_dma_abort();
      break;
    }
    tight_loop_contents();
  }
  /* Keeps the compiler from hoisting a s_line[] store above the completion,
   * the way dma_channel_wait_for_finish_blocking() does. */
  __compiler_memory_barrier();
  s_dma_busy = 0;
}

void St7789Explorer_GetStalls(uint32_t *dma, uint32_t *abort, uint32_t *spi) {
  if (dma != NULL)
    *dma = s_stall_dma;
  if (abort != NULL)
    *abort = s_stall_abort;
  if (spi != NULL)
    *spi = s_stall_spi;
}

/* count is in 16-bit frames. incr=0 repeats one word (solid fill). */
static void st_dma_send16(const void *src, uint count, bool incr) {
  dma_channel_config c;
  if (s_dma_ch < 0 || count == 0)
    return;
  c = dma_channel_get_default_config((uint)s_dma_ch);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
  channel_config_set_read_increment(&c, incr);
  channel_config_set_write_increment(&c, false);
  channel_config_set_dreq(&c, spi_get_dreq(spi0, true));
  dma_channel_configure((uint)s_dma_ch, &c, &spi_get_hw(spi0)->dr, src, count,
                        true);
  s_dma_busy = 1;
}

/* TX-only DMA leaves the RX FIFO full and the overrun flag set; clear both so
 * the next 8-bit spi_write_blocking() (a command) starts from a clean FIFO. */
static void st_spi_drain_rx(void) {
  int guard = 16; /* the PL022 RX FIFO is 8 deep; 16 is a hard stop, not a wait */
  while (spi_is_readable(spi0) && guard-- > 0)
    (void)spi_get_hw(spi0)->dr;
  spi_get_hw(spi0)->icr = SPI_SSPICR_RORIC_BITS | SPI_SSPICR_RTIC_BITS;
}

static void st_cmd(uint8_t cmd, const uint8_t *data, size_t n) {
  gpio_put(PIN_CS, 0);
  gpio_put(PIN_DC, 0);
  st_write(&cmd, 1);
  if (n > 0) {
    gpio_put(PIN_DC, 1);
    st_write(data, n);
  }
  gpio_put(PIN_CS, 1);
}

static void st_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  uint16_t x1 = (uint16_t)(x + w - 1);
  uint16_t y1 = (uint16_t)(y + h - 1);
  uint8_t caset[4] = {(uint8_t)(x >> 8), (uint8_t)x, (uint8_t)(x1 >> 8),
                      (uint8_t)x1};
  uint8_t raset[4] = {(uint8_t)(y >> 8), (uint8_t)y, (uint8_t)(y1 >> 8),
                      (uint8_t)y1};
  st_cmd(ST7789_CASET, caset, 4);
  st_cmd(ST7789_RASET, raset, 4);
}

static void st_backlight(uint8_t brightness) {
  float t = (float)brightness / 255.0f;
  uint16_t value = (uint16_t)(t * t * t * 65535.0f + 0.5f);
  pwm_set_gpio_level(PIN_BL, value);
}

void St7789Explorer_Init(void) {
  s_spi_hz = spi_init(spi0, kSpiHz);
  if (s_dma_ch < 0)
    s_dma_ch = (int)dma_claim_unused_channel(true);
  gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
  gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

  gpio_init(PIN_CS);
  gpio_set_dir(PIN_CS, GPIO_OUT);
  gpio_put(PIN_CS, 1);

  gpio_init(PIN_DC);
  gpio_set_dir(PIN_DC, GPIO_OUT);
  gpio_put(PIN_DC, 0);

  pwm_config pwm_cfg = pwm_get_default_config();
  pwm_set_wrap(pwm_gpio_to_slice_num(PIN_BL), 65535);
  pwm_init(pwm_gpio_to_slice_num(PIN_BL), &pwm_cfg, true);
  gpio_set_function(PIN_BL, GPIO_FUNC_PWM);
  st_backlight(255);

  st_cmd(ST7789_SWRESET, NULL, 0);
  sleep_ms(150);

  st_cmd(ST7789_TEON, (const uint8_t *)"\x00", 1);
  st_cmd(ST7789_COLMOD, (const uint8_t *)"\x05", 1);
  st_cmd(ST7789_PORCTRL, (const uint8_t *)"\x0c\x0c\x00\x33\x33", 5);
  st_cmd(ST7789_GCTRL, (const uint8_t *)"\x14", 1);
  st_cmd(ST7789_VCOMS, (const uint8_t *)"\x37", 1);
  st_cmd(ST7789_LCMCTRL, (const uint8_t *)"\x2c", 1);
  st_cmd(ST7789_VDVVRHEN, (const uint8_t *)"\x01", 1);
  st_cmd(ST7789_VRHS, (const uint8_t *)"\x12", 1);
  st_cmd(ST7789_VDVS, (const uint8_t *)"\x20", 1);
  st_cmd(ST7789_PWCTRL1, (const uint8_t *)"\xa4\xa1", 2);
  st_cmd(ST7789_PVGAMCTRL,
         (const uint8_t *)"\xD0\x08\x11\x08\x0c\x15\x39\x33\x50\x36\x13\x14\x29\x2d",
         14);
  st_cmd(ST7789_NVGAMCTRL,
         (const uint8_t *)"\xD0\x08\x10\x08\x06\x06\x39\x44\x51\x0b\x16\x14\x2f\x31",
         14);
  st_cmd(ST7789_STE, (const uint8_t *)"\x01\x2C", 2);
  st_cmd(ST7789_FRCTRL2, (const uint8_t *)"\x15", 1);
  st_cmd(ST7789_INVON, NULL, 0);
  st_cmd(ST7789_SLPOUT, NULL, 0);
  st_cmd(ST7789_DISPON, NULL, 0);
  sleep_ms(100);

  st_window(0, 0, kSt7789ExplorerWidth, kSt7789ExplorerHeight);

  /* MADCTL D3 is the RGB/BGR select and 1 means BGR, so the MADCTL_RGB macro
   * actually asks for BGR. We send RGB565, so leave D3 clear. */
  uint8_t madctl = (uint8_t)(MADCTL_COL_ORDER | MADCTL_SWAP_XY |
                             MADCTL_SCAN_ORDER);
  st_cmd(ST7789_MADCTL, &madctl, 1);

  /* The boot log, not the comment above kSpiHz, is the authority on the rate
   * the PL022 dividers actually reached. */
  s_stall_dma = 0;
  s_stall_abort = 0;
  s_stall_spi = 0;
  printf("st7789: spi req=%u actual=%u Hz dma_ch=%d 16bit frames wait_to=%uus\n",
         (unsigned)kSpiHz, (unsigned)s_spi_hz, s_dma_ch,
         (unsigned)kStWaitTimeoutUs);
}

void St7789Explorer_Fill(uint16_t color) {
  int y;
  St7789Explorer_BeginFrame();
  for (y = 0; y < kSt7789ExplorerHeight; y++)
    St7789Explorer_WriteSolidLine(color, kSt7789ExplorerWidth);
  St7789Explorer_EndFrame();
}

void St7789Explorer_BeginFrame(void) {
  uint8_t ramwr = ST7789_RAMWR;
  st_window(0, 0, kSt7789ExplorerWidth, kSt7789ExplorerHeight);
  gpio_put(PIN_CS, 0);
  gpio_put(PIN_DC, 0);
  st_write(&ramwr, 1);
  gpio_put(PIN_DC, 1);
  /* Payload only: the command above had to go out as 8-bit frames. */
  st_spi_idle();
  spi_set_format(spi0, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
  s_line_idx = 0;
}

void St7789Explorer_WriteRgb565Line(const uint16_t *px, int count) {
  uint16_t *dst;
  if (count > kSt7789ExplorerWidth)
    count = kSt7789ExplorerWidth;
  if (count <= 0)
    return;
  /* Fill the buffer the DMA is NOT reading, then hand it over. */
  dst = s_line[s_line_idx];
  memcpy(dst, px, (size_t)count * sizeof(uint16_t));
  st_dma_wait();
  st_dma_send16(dst, (uint)count, true);
  s_line_idx ^= 1;
}

void St7789Explorer_WriteSolidLine(uint16_t color, int count) {
  if (count > kSt7789ExplorerWidth)
    count = kSt7789ExplorerWidth;
  if (count <= 0)
    return;
  st_dma_wait();
  s_solid_word = color;
  st_dma_send16(&s_solid_word, (uint)count, false);
}

void St7789Explorer_EndFrame(void) {
  st_dma_wait();
  st_spi_idle();
  /* CS goes high before the format switch, so the only point in the frame
   * where the PL022 is disabled while the panel is selected is the 8->16 bit
   * switch in BeginFrame. (SCK idles low with CPOL=0 and no edge is generated
   * by toggling SSE, so that one is expected to be harmless; if the panel ever
   * shears by one bit, that switch is the first thing to move outside CS.) */
  gpio_put(PIN_CS, 1);
  spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
  st_spi_drain_rx();
}
