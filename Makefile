TARGET_EXEC := sm_rev
MINI_TARGET_EXEC := sm_rev_mini

PYTHON := /usr/bin/env python3
CFLAGS := $(if $(CFLAGS),$(CFLAGS),-O2 -fno-strict-aliasing -Werror)

# Native macOS build: links SDL2 as a framework, enables asset bundling by default
# Usage: make NATIVE_MAC=1
NATIVE_MAC ?= 0

ifeq ($(NATIVE_MAC),1)
  CFLAGS += -F/Library/Frameworks -I/Library/Frameworks/SDL2.framework/Headers \
            -F$(HOME)/Library/Frameworks -I$(HOME)/Library/Frameworks/SDL2.framework/Headers \
            -DSYSTEM_VOLUME_MIXER_AVAILABLE=0 -I.
else
  CFLAGS += $(shell sdl2-config --cflags) -DSYSTEM_VOLUME_MIXER_AVAILABLE=0 -I.
endif

# Optional: Bundle ROM and config into binary (make BUNDLE_ASSETS=1)
# Set ROM_FILE and CONFIG_FILE to the paths of your files
ROM_FILE ?= sm.smc
CONFIG_FILE ?= sm.ini
BUNDLE_ASSETS ?= 0

ifeq ($(NATIVE_MAC),1)
  BUNDLE_ASSETS := 1
endif

ifeq ($(BUNDLE_ASSETS),1)
  CFLAGS += -DBUNDLE_ASSETS
  EMBEDDED_SRCS := src/embedded/rom_data.c src/embedded/config_data.c
  EMBEDDED_OBJS := $(EMBEDDED_SRCS:%.c=%.o)
endif

FULL_SRCS := $(filter-out $(wildcard src/mini_*.c) src/stubs_mini.c,$(wildcard src/*.c)) \
             $(wildcard src/snes/*.c) \
             third_party/gl_core/gl_core_3_1.c \
             third_party/cJSON.c \
             $(EMBEDDED_SRCS)
OBJS := $(FULL_SRCS:%.c=%.o)

MINI_RUNTIME_SRCS := $(wildcard src/mini_*.c)
MINI_SUPPORT_SRCS := src/stubs_mini.c \
                     $(filter-out src/main.c src/opengl.c src/glsl_shader.c src/sm_cpu_infra.c src/sm_rtl.c $(wildcard src/mini_*.c) src/stubs_mini.c,$(wildcard src/*.c))
MINI_EXTRA_SRCS := third_party/cJSON.c
MINI_SRCS := $(MINI_RUNTIME_SRCS) $(MINI_SUPPORT_SRCS) $(MINI_EXTRA_SRCS)
MINI_CFLAGS = $(CFLAGS) -DCURRENT_BUILD=BUILD_MINI -ffunction-sections -fdata-sections
MINI_LDFLAGS = $(LDFLAGS) $(SDLFLAGS) -Wl,--gc-sections

ifeq ($(BUNDLE_ASSETS),1)
  # Regenerate embedded files if sources are newer
  src/embedded/rom_data.c: $(ROM_FILE) scripts/file2c.py
	@mkdir -p src/embedded
	$(PYTHON) scripts/file2c.py $(ROM_FILE) $@ sm_rom
  src/embedded/config_data.c: $(CONFIG_FILE) scripts/file2c.py
	@mkdir -p src/embedded
	$(PYTHON) scripts/file2c.py $(CONFIG_FILE) $@ sm_config
  src/embedded/%.o: src/embedded/%.c
	$(CC) -c $(CFLAGS) $< -o $@
endif

ifeq (${OS},Windows_NT)
    WINDRES := windres
    SDLFLAGS := -Wl,-Bstatic $(shell sdl2-config --static-libs)
else ifeq ($(NATIVE_MAC),1)
    SDLFLAGS := -framework SDL2 -lm
else
    SDLFLAGS := $(shell sdl2-config --libs) -lm
endif

.PHONY: all clean clean_obj run test test-fast mini mini-test mini-mac

all: $(TARGET_EXEC)

$(TARGET_EXEC): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS) $(SDLFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

mini: $(MINI_TARGET_EXEC)

$(MINI_TARGET_EXEC): $(MINI_SRCS)
	$(CC) $(MINI_CFLAGS) $^ -o $@ $(MINI_LDFLAGS)

run: all
	./$(TARGET_EXEC)

mini-test: mini
	./$(MINI_TARGET_EXEC) --headless --frames 3

mini-mac: NATIVE_MAC=1
mini-mac: mini

clean: clean_obj
clean_obj:
	@$(RM) $(OBJS) $(TARGET_EXEC) $(MINI_TARGET_EXEC) src/embedded/*.o src/embedded/*.c

test: all
	$(PYTHON) tests/run_tests.py -v

test-fast: all
	$(PYTHON) tests/run_tests.py --fast -v
