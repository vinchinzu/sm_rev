#include "hardware/spi.h"
#include "pico/stdlib.h"

#include <stdio.h>

/* Pico Explorer ST7789 240x240 smoke test. No kernel, no ROM.
 * Official pins: CS=17 DC=16 SCK=18 MOSI=19, no RESET (AP2502 backlight).
 * GP20 is Explorer I2C SDA — do not drive it as backlight.
 * EXPLORER_FALLBACK_PINS: CircuitPython demo DC=20 RESET=21.
 * Bad init is recoverable: hold BOOTSEL, unplug, replug. */

enum {
  kWidth = 240,
  kHeight = 240,
  kColStart = 0,
  kRowStart = 80, /* 240x240 window on a 240x320 panel, rotation 0 */
  kSpiHz = 32000000,
  kBlock = 8,
};

#define PIN_SCK 18
#define PIN_MOSI 19
#define PIN_CS 17

#ifdef EXPLORER_FALLBACK_PINS
#define PIN_DC 20
#define PIN_RESET 21
#else
#define PIN_DC 16
#define PIN_RESET (-1)
#endif

#define ST7789_SWRESET 0x01
#define ST7789_SLPOUT 0x11
#define ST7789_NORON 0x13
#define ST7789_INVON 0x21
#define ST7789_DISPON 0x29
#define ST7789_CASET 0x2A
#define ST7789_RASET 0x2B
#define ST7789_RAMWR 0x2C
#define ST7789_MADCTL 0x36
#define ST7789_COLMOD 0x3A

#define MADCTL_MX 0x40
#define MADCTL_MY 0x80

#define RGB565(r, g, b) \
  ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xF8) >> 3)))

#define COLOR_RED RGB565(255, 0, 0)
#define COLOR_GREEN RGB565(0, 255, 0)
#define COLOR_BLUE RGB565(0, 0, 255)
#define COLOR_WHITE RGB565(255, 255, 255)
#define COLOR_CYAN RGB565(0, 255, 255)

static void st_write(const uint8_t *data, size_t n) {
  spi_write_blocking(spi0, data, n);
}

static void st_cmd(uint8_t cmd) {
  gpio_put(PIN_DC, 0);
  gpio_put(PIN_CS, 0);
  st_write(&cmd, 1);
  gpio_put(PIN_CS, 1);
}

static void st_cmd_data(uint8_t cmd, const uint8_t *data, size_t n) {
  gpio_put(PIN_DC, 0);
  gpio_put(PIN_CS, 0);
  st_write(&cmd, 1);
  if (n > 0) {
    gpio_put(PIN_DC, 1);
    st_write(data, n);
  }
  gpio_put(PIN_CS, 1);
}

static void st_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  uint16_t xs = (uint16_t)(x0 + kColStart);
  uint16_t xe = (uint16_t)(x1 + kColStart);
  uint16_t ys = (uint16_t)(y0 + kRowStart);
  uint16_t ye = (uint16_t)(y1 + kRowStart);
  uint8_t caset[4] = {(uint8_t)(xs >> 8), (uint8_t)xs, (uint8_t)(xe >> 8),
                      (uint8_t)xe};
  uint8_t raset[4] = {(uint8_t)(ys >> 8), (uint8_t)ys, (uint8_t)(ye >> 8),
                      (uint8_t)ye};
  st_cmd_data(ST7789_CASET, caset, 4);
  st_cmd_data(ST7789_RASET, raset, 4);
}

static void put_rgb565(uint8_t *dst, uint16_t color) {
  dst[0] = (uint8_t)(color >> 8);
  dst[1] = (uint8_t)color;
}

static uint16_t bar_color(int x) {
  if (x < 60)
    return COLOR_RED;
  if (x < 120)
    return COLOR_GREEN;
  if (x < 180)
    return COLOR_BLUE;
  return COLOR_WHITE;
}

static void draw_frame(int bx, int by) {
  static uint8_t line[kWidth * 2];
  uint8_t ramwr = ST7789_RAMWR;
  int y;
  int x;

  st_window(0, 0, kWidth - 1, kHeight - 1);
  gpio_put(PIN_DC, 0);
  gpio_put(PIN_CS, 0);
  st_write(&ramwr, 1);
  gpio_put(PIN_DC, 1);

  for (y = 0; y < kHeight; y++) {
    for (x = 0; x < kWidth; x++) {
      uint16_t c;
      if (y == 0 || y == kHeight - 1 || x == 0 || x == kWidth - 1)
        c = COLOR_WHITE;
      else if (x >= bx && x < bx + kBlock && y >= by && y < by + kBlock)
        c = COLOR_CYAN;
      else
        c = bar_color(x);
      put_rgb565(&line[x * 2], c);
    }
    st_write(line, sizeof(line));
  }
  gpio_put(PIN_CS, 1);
}

static void st_hw_reset(void) {
#if PIN_RESET >= 0
  gpio_init(PIN_RESET);
  gpio_set_dir(PIN_RESET, GPIO_OUT);
  gpio_put(PIN_RESET, 1);
  sleep_ms(10);
  gpio_put(PIN_RESET, 0);
  sleep_ms(20);
  gpio_put(PIN_RESET, 1);
  sleep_ms(150);
#endif
}

static void st_init(void) {
  uint8_t colmod = 0x55; /* 16 bpp */
  uint8_t madctl = (uint8_t)(MADCTL_MX | MADCTL_MY); /* rotation 0, RGB */

  spi_init(spi0, kSpiHz);
  gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
  gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

  gpio_init(PIN_CS);
  gpio_set_dir(PIN_CS, GPIO_OUT);
  gpio_put(PIN_CS, 1);

  gpio_init(PIN_DC);
  gpio_set_dir(PIN_DC, GPIO_OUT);
  gpio_put(PIN_DC, 0);

  st_hw_reset();

  st_cmd(ST7789_SWRESET);
  sleep_ms(150);
  st_cmd(ST7789_SLPOUT);
  sleep_ms(120);
  st_cmd_data(ST7789_COLMOD, &colmod, 1);
  sleep_ms(10);
  st_cmd_data(ST7789_MADCTL, &madctl, 1);
  st_cmd(ST7789_INVON);
  st_cmd(ST7789_NORON);
  sleep_ms(10);
  st_cmd(ST7789_DISPON);
  sleep_ms(20);
}

int main(void) {
  int x = 1;
  int y = 1;
  int dx = 2;
  int dy = 2;

  stdio_init_all();
  sleep_ms(1500);

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  st_init();
  printf("explorer-st7789 ok cs=%d dc=%d reset=%d rowstart=%d 240x240\n", PIN_CS,
         PIN_DC, PIN_RESET, kRowStart);

  for (;;) {
    draw_frame(x, y);
    gpio_xor_mask(1u << PICO_DEFAULT_LED_PIN);
    x += dx;
    y += dy;
    if (x <= 1 || x + kBlock >= kWidth - 1)
      dx = -dx;
    if (y <= 1 || y + kBlock >= kHeight - 1)
      dy = -dy;
    printf("explorer-st7789 ok x=%d y=%d\n", x, y);
    sleep_ms(16);
  }
}
