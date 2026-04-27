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
            -DSYSTEM_VOLUME_MIXER_AVAILABLE=0 -I. -iquote src
else
  CFLAGS += $(shell sdl2-config --cflags) -DSYSTEM_VOLUME_MIXER_AVAILABLE=0 -I. -iquote src
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

FULL_SRCS := $(wildcard src/*.c) \
             $(wildcard src/snes/*.c) \
             third_party/gl_core/gl_core_3_1.c \
             third_party/cJSON.c \
             $(EMBEDDED_SRCS)
OBJS := $(FULL_SRCS:%.c=%.o)

MINI_RUNTIME_SRCS := $(wildcard src/mini/*.c)
# Mini now links the shared gameplay engine and constrains content at runtime to
# Landing Site. Keep only the full host, emulator bridge, and GL frontend out.
MINI_SHARED_ENGINE_SRCS := $(filter-out src/main.c src/opengl.c src/glsl_shader.c src/sm_cpu_infra.c src/sm_rtl.c,$(wildcard src/*.c))
MINI_EXTRA_SRCS := third_party/cJSON.c
MINI_SRCS := $(MINI_RUNTIME_SRCS) $(MINI_SHARED_ENGINE_SRCS) $(MINI_EXTRA_SRCS)
MINI_KERNEL_RUNTIME_SRCS := $(filter-out src/mini/mini_main.c src/mini/mini_runtime.c src/mini/mini_renderer.c src/mini/mini_record.c src/mini/mini_input_script.c src/mini/mini_backdrop.c,$(MINI_RUNTIME_SRCS))
MINI_KERNEL_SRCS := $(MINI_KERNEL_RUNTIME_SRCS) $(MINI_SHARED_ENGINE_SRCS) $(MINI_EXTRA_SRCS)
MINI_KERNEL_OBJS := $(MINI_KERNEL_SRCS:%.c=%.mini.o)
MINI_KERNEL_LIB := libsm_rev_mini_kernel.a
MINI_ROLLBACK_TEST := sm_rev_mini_rollback_test
MINI_RUST_HOST := sm_rev_mini_rs
MINI_ASSET_DEPS := src/mini/mini_generated_background_data.inc
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

.PHONY: all clean clean_obj run test test-fast mini mini-test mini-mac mini-rollback-test mini-rust-host

all: $(TARGET_EXEC)

$(TARGET_EXEC): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS) $(SDLFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

mini: $(MINI_TARGET_EXEC)

$(MINI_TARGET_EXEC): $(MINI_SRCS) $(MINI_ASSET_DEPS)
	$(CC) $(MINI_CFLAGS) $(MINI_SRCS) -o $@ $(MINI_LDFLAGS)

%.mini.o: %.c
	$(CC) -c $(MINI_CFLAGS) $< -o $@

$(MINI_KERNEL_LIB): $(MINI_KERNEL_OBJS) $(MINI_ASSET_DEPS)
	$(AR) rcs $@ $(MINI_KERNEL_OBJS)

mini-rollback-test: $(MINI_ROLLBACK_TEST)
	./$(MINI_ROLLBACK_TEST)

$(MINI_ROLLBACK_TEST): tests/mini_rollback_api.c $(MINI_KERNEL_LIB)
	$(CC) $(MINI_CFLAGS) $< -o $@ -L. -lsm_rev_mini_kernel $(MINI_LDFLAGS)

mini-rust-host: $(MINI_RUST_HOST)

$(MINI_RUST_HOST): src/mini/mini_rust_host.rs $(MINI_KERNEL_LIB)
	rustc --edition=2021 $< -o $@ -L. -l static=sm_rev_mini_kernel -C link-arg=-lm

run: all
	./$(TARGET_EXEC)

mini-test: mini mini-rollback-test
	./$(MINI_TARGET_EXEC) --headless --frames 3

mini-mac: NATIVE_MAC=1
mini-mac: mini

clean: clean_obj
clean_obj:
	@$(RM) $(OBJS) $(TARGET_EXEC) $(MINI_TARGET_EXEC) $(MINI_KERNEL_OBJS) $(MINI_KERNEL_LIB) $(MINI_ROLLBACK_TEST) $(MINI_RUST_HOST) src/embedded/*.o src/embedded/*.c

test: all
	$(PYTHON) tests/run_tests.py -v

test-fast: all
	$(PYTHON) tests/run_tests.py --fast -v
