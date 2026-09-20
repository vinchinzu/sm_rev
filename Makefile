TARGET_EXEC := sm_rev
MINI_TARGET_EXEC := sm_rev_mini
MODDABLE_TARGET_EXEC := sm_rev_moddable

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

CORE_SRCS := $(wildcard src/*.c)
HOST_SRCS := $(wildcard src/host/*.c)
SNES_SRCS := $(wildcard src/snes/*.c)
FULL_THIRD_PARTY_SRCS := third_party/gl_core/gl_core_3_1.c third_party/cJSON.c

FULL_SRCS := $(filter-out src/predict_cli.c,$(CORE_SRCS)) \
             $(HOST_SRCS) \
             $(SNES_SRCS) \
             $(FULL_THIRD_PARTY_SRCS) \
             $(EMBEDDED_SRCS)
OBJS := $(FULL_SRCS:%.c=%.o)

MINI_RUNTIME_SRCS := $(wildcard src/mini/*.c)
# Mini now links the shared gameplay engine and constrains content at runtime to
# Ceres plus Landing Site. Keep only the full host and emulator bridge out.
# Also exclude predict_cli.c which has its own main()
MINI_SHARED_ENGINE_SRCS := $(filter-out src/main.c src/sm_cpu_infra.c src/sm_rtl.c src/predict_cli.c,$(CORE_SRCS))
MINI_EXTRA_SRCS := third_party/cJSON.c
MINI_SRCS := $(MINI_RUNTIME_SRCS) $(MINI_SHARED_ENGINE_SRCS) $(MINI_EXTRA_SRCS)
MINI_KERNEL_RUNTIME_SRCS := $(filter-out src/mini/mini_main.c src/mini/mini_runtime.c src/mini/mini_renderer.c src/mini/mini_record.c src/mini/mini_input_script.c src/mini/mini_backdrop.c,$(MINI_RUNTIME_SRCS))
MINI_KERNEL_SRCS := $(MINI_KERNEL_RUNTIME_SRCS) $(MINI_SHARED_ENGINE_SRCS) $(MINI_EXTRA_SRCS)
MINI_KERNEL_OBJS := $(MINI_KERNEL_SRCS:%.c=%.mini.o)
MINI_KERNEL_LIB := libsm_rev_mini_kernel.a
MINI_BROWSER_SRCS := $(MINI_KERNEL_SRCS) src/mini/mini_renderer.c
MINI_BROWSER_LIB := libsm_rev_mini_net.so
MINI_ROLLBACK_TEST := sm_rev_mini_rollback_test
MINI_PREDICT_TEST := sm_rev_mini_predict_test
MINI_PREDICT_GOLDEN := sm_rev_mini_predict_golden
MINI_WRAM_PEEK_TEST := sm_rev_mini_wram_peek_test
MINI_PREDICT_CLI := sm_rev_predict
MINI_MSS_FIXTURE_GEN := generate_mss_fixture
MINI_ENEMY_OBS_TEST := sm_rev_mini_enemy_obs_test
MINI_RUST_HOST := sm_rev_mini_rs
MINI_ASSET_DEPS := src/mini/mini_generated_background_data.inc
MINI_CFLAGS = $(CFLAGS) -DCURRENT_BUILD=BUILD_MINI -ffunction-sections -fdata-sections
MINI_LDFLAGS = $(LDFLAGS) $(SDLFLAGS) -Wl,--gc-sections
MODDABLE_SRCS := $(MINI_SRCS)
MODDABLE_CFLAGS = $(CFLAGS) -DCURRENT_BUILD=BUILD_MODDABLE -ffunction-sections -fdata-sections
MODDABLE_LDFLAGS = $(MINI_LDFLAGS)

# Pico kernel is a sideline. Do not reuse CFLAGS/MINI_CFLAGS (those pull SDL).
# Separate objects (*.pico.o) so this never clobbers *.o / *.mini.o / sm_rev_mini.
# PICO_KERNEL_SRCS comes from src/pico/pico_kernel_sources.mk (ship1 + mini KEEP).
# Do not fall back to MINI_KERNEL_SRCS if that include fails.
-include src/pico/pico_kernel_sources.mk
PICO_CFLAGS := -O2 -fno-strict-aliasing -Werror -DSYSTEM_VOLUME_MIXER_AVAILABLE=0 -I. -iquote src -iquote src/mini -DCURRENT_BUILD=BUILD_PICO -ffunction-sections -fdata-sections
PICO_LDFLAGS := -lm -Wl,--gc-sections
PICO_TARGET_EXEC := sm_rev_pico_kernel
PICO_KERNEL_TEST := sm_rev_pico_kernel_test
PICO_MOVE_TEST := sm_rev_pico_move_tileset_test
PICO_LS_LAYERS_TEST := sm_rev_pico_ls_layers_test
PICO_FEEL_TEST := sm_rev_pico_feel_test
PICO_EXPLORER_BUTTONS_TEST := sm_rev_pico_explorer_buttons_test
PICO_SAMUS_ANIM_LR_TEST := sm_rev_pico_samus_anim_lr_test
PICO_MORPH_TEST := sm_rev_pico_morph_input_test
PICO_KERNEL_LIB := libsm_rev_pico_kernel.a
PICO_SDL_EXCLUDE_SRCS := src/config.c src/default_controls.c src/mini/mini_editor_path.c
PICO_KERNEL_LIB_SRCS := $(filter-out $(PICO_SDL_EXCLUDE_SRCS),$(PICO_KERNEL_SRCS)) \
  src/pico/pico_config_stub.c \
  src/pico/pico_editor_path.c \
  src/pico/pico_stubs.c \
  third_party/cJSON.c
PICO_KERNEL_LIB_OBJS := $(PICO_KERNEL_LIB_SRCS:%.c=%.pico.o)

# RP2350 / Pico 2 sideline. Objects live in build/pico2/ (never *.pico.o / *.mini.o).
PICO_SDK_PATH ?= /home/v/01_projects/13_hardware/pico/pico-sdk
PICO2_BUILD_DIR := build/pico2
PICO2_CMAKE_DIR := src/pico/rp2350
PICO2_TARGET := sm_rev_pico2
# Explorer ST7789 display-test sideline. Objects live in build/pico2-explorer/.
PICO2_EXPLORER_BUILD_DIR := build/pico2-explorer
PICO2_EXPLORER_CMAKE_DIR := src/pico/rp2350/explorer
PICO2_EXPLORER_TARGET := sm_rev_pico_explorer_test
PICO2_PICOTOOL_DIR ?= $(firstword $(wildcard $(HOME)/.local/lib/cmake/picotool) /home/v/01_projects/13_hardware/pico/gameboy/build/_deps/picotool)
PICO2_PIOASM_DIR ?= /home/v/01_projects/13_hardware/pico/gameboy/build/pioasm-install/pioasm
PICO2_SKIP_PACKAGES := extra/arm-none-eabi-gcc extra/arm-none-eabi-newlib extra/arm-none-eabi-binutils
PICO2_ARM_GCC ?= $(shell command -v arm-none-eabi-gcc 2>/dev/null)
ifeq ($(PICO2_ARM_GCC),)
PICO2_ARM_GCC := $(wildcard $(HOME)/.local/share/mise/installs/gcc-arm-none-eabi/latest/bin/arm-none-eabi-gcc)
endif
ifneq ($(PICO2_ARM_GCC),)
PICO2_ARM_GCC_DIR := $(dir $(PICO2_ARM_GCC))
endif

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

.PHONY: all clean clean_obj run test test-fast mini mini-test mini-mac mini-rollback-test mini-predict-test mini-predict-golden mini-wram-peek-test mini-predict-cli mini-rust-host mini-browser-lib mini-browser-server moddable moddable-test mini-enemy-obs-test mini-enemy-hookup-test mini-cli-enemy-test mini-emu-residual hm-test pico-kernel pico-kernel-test pico-kernel-size pico-kernel-rp2350 pico-explorer-test pico-move-test pico-ls-layers-test pico-feel-test pico-explorer-buttons-test pico-samus-anim-lr-test pico-morph-test pico-picotool pico-flash

all: $(TARGET_EXEC)

$(TARGET_EXEC): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS) $(SDLFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

mini: $(MINI_TARGET_EXEC)

$(MINI_TARGET_EXEC): $(MINI_SRCS) $(MINI_ASSET_DEPS)
	$(CC) $(MINI_CFLAGS) $(MINI_SRCS) -o $@ $(MINI_LDFLAGS)

moddable: $(MODDABLE_TARGET_EXEC)

$(MODDABLE_TARGET_EXEC): $(MODDABLE_SRCS) $(MINI_ASSET_DEPS)
	$(CC) $(MODDABLE_CFLAGS) $(MODDABLE_SRCS) -o $@ $(MODDABLE_LDFLAGS)

%.mini.o: %.c
	$(CC) -c $(MINI_CFLAGS) $< -o $@

$(MINI_KERNEL_LIB): $(MINI_KERNEL_OBJS) $(MINI_ASSET_DEPS)
	$(AR) rcs $@ $(MINI_KERNEL_OBJS)

mini-browser-lib: $(MINI_BROWSER_LIB)

$(MINI_BROWSER_LIB): $(MINI_BROWSER_SRCS) src/mini/mini_net_bridge.h src/mini/mini_renderer.h $(MINI_ASSET_DEPS)
	$(CC) $(MINI_CFLAGS) -fPIC -fvisibility=hidden -shared $(MINI_BROWSER_SRCS) -o $@ $(MINI_LDFLAGS)

mini-browser-server: mini-browser-lib
	$(PYTHON) tools/mini_browser_server.py

mini-rollback-test: $(MINI_ROLLBACK_TEST)
	./$(MINI_ROLLBACK_TEST)

$(MINI_ROLLBACK_TEST): tests/mini_rollback_api.c $(MINI_KERNEL_LIB)
	$(CC) $(MINI_CFLAGS) $< -o $@ -L. -lsm_rev_mini_kernel $(MINI_LDFLAGS)

# mini-predict-test target removed: mini_predict_api.c deleted (superseded by mini_predict_golden.c)

mini-predict-golden: $(MINI_PREDICT_GOLDEN)
	./$(MINI_PREDICT_GOLDEN)

$(MINI_PREDICT_GOLDEN): tests/mini_predict_golden.c tests/mini_test_room.c $(MINI_KERNEL_LIB)
	$(CC) $(MINI_CFLAGS) tests/mini_predict_golden.c tests/mini_test_room.c -o $@ -L. -lsm_rev_mini_kernel $(MINI_LDFLAGS)

mini-wram-peek-test: $(MINI_WRAM_PEEK_TEST)
	./$(MINI_WRAM_PEEK_TEST)

$(MINI_WRAM_PEEK_TEST): tests/test_wram_peek_load.c $(MINI_KERNEL_LIB)
	$(CC) $(MINI_CFLAGS) $< -o $@ -L. -lsm_rev_mini_kernel $(MINI_LDFLAGS)

$(MINI_MSS_FIXTURE_GEN): tests/generate_mss_fixture.c $(MINI_KERNEL_LIB)
	$(CC) $(MINI_CFLAGS) $< -o $@ -L. -lsm_rev_mini_kernel $(MINI_LDFLAGS)

mini-predict-cli: $(MINI_PREDICT_CLI)

$(MINI_PREDICT_CLI): src/predict_cli.c $(MINI_KERNEL_LIB)
	$(CC) $(MINI_CFLAGS) $< -o $@ -L. -lsm_rev_mini_kernel $(MINI_LDFLAGS)

mini-enemy-obs-test: $(MINI_ENEMY_OBS_TEST)
	./$(MINI_ENEMY_OBS_TEST)

$(MINI_ENEMY_OBS_TEST): tests/test_enemy_observation.c $(MINI_KERNEL_LIB)
	$(CC) $(MINI_CFLAGS) $< -o $@ -L. -lsm_rev_mini_kernel $(MINI_LDFLAGS)

mini-enemy-hookup-test: tests/test_enemy_hookup.c $(MINI_KERNEL_LIB)
	$(CC) $(MINI_CFLAGS) $< -o sm_rev_mini_enemy_hookup_test -L. -lsm_rev_mini_kernel $(MINI_LDFLAGS)
	./sm_rev_mini_enemy_hookup_test

mini-cli-enemy-test: $(MINI_PREDICT_CLI)
	python3 tests/test_cli_enemy_prediction.py

mini-rust-host: $(MINI_RUST_HOST)

$(MINI_RUST_HOST): src/mini/mini_rust_host.rs $(MINI_KERNEL_LIB)
	rustc --edition=2021 $< -o $@ -L. -l static=sm_rev_mini_kernel -C link-arg=-lm

run: all
	./$(TARGET_EXEC)

mini-test: mini mini-rollback-test mini-predict-golden mini-wram-peek-test mini-predict-cli
	./$(MINI_TARGET_EXEC) --headless --frames 3
	python3 tests/test_load_state_cli.py

mini-emu-residual: all mini
	$(PYTHON) tools/residual_profile.py

hm-test: mini-predict-cli
	cd physics-hs && HM_REQUIRED=1 PATH="$(HOME)/.ghcup/bin:$$PATH" cabal test --test-show-details=direct

moddable-test: moddable
	./$(MODDABLE_TARGET_EXEC) --headless --frames 3

mini-mac: NATIVE_MAC=1
mini-mac: mini

pico-kernel: $(PICO_TARGET_EXEC)

%.pico.o: %.c
	$(CC) -c $(PICO_CFLAGS) $< -o $@

src/pico/pico_stubs.pico.o: src/pico/pico_stubs.c src/pico/pico_stubs_generated.inc
	$(CC) -c $(PICO_CFLAGS) $< -o $@

$(PICO_KERNEL_LIB): $(PICO_KERNEL_LIB_OBJS) $(MINI_ASSET_DEPS)
	$(if $(PICO_KERNEL_SRCS),,$(error PICO_KERNEL_SRCS is not defined; failed to include src/pico/pico_kernel_sources.mk (do not fall back to MINI_KERNEL_SRCS)))
	@$(RM) $@
	$(AR) rcs $@ $(PICO_KERNEL_LIB_OBJS)

$(PICO_TARGET_EXEC): src/pico/pico_kernel_main.c $(PICO_KERNEL_LIB)
	$(if $(PICO_KERNEL_SRCS),,$(error PICO_KERNEL_SRCS is not defined; failed to include src/pico/pico_kernel_sources.mk (do not fall back to MINI_KERNEL_SRCS)))
	$(CC) $(PICO_CFLAGS) src/pico/pico_kernel_main.c -o $@ -L. -lsm_rev_pico_kernel $(PICO_LDFLAGS)

pico-kernel-test: $(PICO_KERNEL_TEST)
	./$(PICO_KERNEL_TEST)

$(PICO_KERNEL_TEST): tests/test_pico_kernel.c $(PICO_KERNEL_LIB)
	$(CC) $(PICO_CFLAGS) tests/test_pico_kernel.c -o $@ -L. -lsm_rev_pico_kernel $(PICO_LDFLAGS)

pico-kernel-size: $(PICO_TARGET_EXEC)
	@echo "=== pico-kernel size (host gcc sideline; not sm_rev_mini) ==="
	size $(PICO_TARGET_EXEC)
	@echo "g_ram=128KiB. Pico game chip: no 4MiB g_mini_rom, no 64KiB g_mini_vram."
	@echo "MiniSaveState full blob is not allocated by pico-kernel main."
	@echo "Leftover BSS owners if over 200KiB: g_mini_editor_tiles4bpp, g_samus_bank92 (display-side GFX)."

# Sideline: not a dependency of mini / all / pico-kernel / pico-kernel-test.
# Cross objects stay in $(PICO2_BUILD_DIR); never overwrite *.pico.o or sm_rev_mini.
$(PICO2_BUILD_DIR)/pico_kernel_lib_srcs.cmake: src/pico/pico_kernel_sources.mk Makefile
	$(if $(PICO_KERNEL_SRCS),,$(error PICO_KERNEL_SRCS is not defined; failed to include src/pico/pico_kernel_sources.mk (do not fall back to MINI_KERNEL_SRCS)))
	@mkdir -p $(PICO2_BUILD_DIR)
	@printf 'set(PICO_KERNEL_LIB_SRCS\n' > $@
	@for f in $(PICO_KERNEL_LIB_SRCS); do printf '  $${SM_REV_ROOT}/%s\n' "$$f"; done >> $@
	@printf ')\n' >> $@

pico-kernel-rp2350: $(PICO2_BUILD_DIR)/pico_kernel_lib_srcs.cmake
ifeq ($(PICO2_ARM_GCC),)
	@echo "skip: pico-kernel-rp2350: arm-none-eabi-gcc not found. Install Arch packages: $(PICO2_SKIP_PACKAGES)"
else ifeq ($(wildcard $(PICO_SDK_PATH)/pico_sdk_init.cmake),)
	@echo "skip: pico-kernel-rp2350: PICO_SDK_PATH missing pico_sdk_init.cmake (tried $(PICO_SDK_PATH)). Install Arch packages: $(PICO2_SKIP_PACKAGES)"
else
	PATH="$(PICO2_ARM_GCC_DIR):$$PATH" PICO_SDK_PATH="$(PICO_SDK_PATH)" cmake -S $(PICO2_CMAKE_DIR) -B $(PICO2_BUILD_DIR) \
	  -DPICO_SDK_PATH="$(PICO_SDK_PATH)" \
	  -DPICO_BOARD=pico2 \
	  -DCMAKE_BUILD_TYPE=Release \
	  $(if $(wildcard $(PICO2_PICOTOOL_DIR)/picotoolConfig.cmake),-Dpicotool_DIR="$(PICO2_PICOTOOL_DIR)") \
	  $(if $(wildcard $(PICO2_PIOASM_DIR)/pioasmConfig.cmake),-Dpioasm_DIR="$(PICO2_PIOASM_DIR)")
	PATH="$(PICO2_ARM_GCC_DIR):$$PATH" cmake --build $(PICO2_BUILD_DIR) --target $(PICO2_TARGET) -j
	@echo "=== pico-kernel-rp2350 size (RP2350 / Pico 2; not host gcc, not sm_rev_mini) ==="
	PATH="$(PICO2_ARM_GCC_DIR):$$PATH" arm-none-eabi-size $(PICO2_BUILD_DIR)/$(PICO2_TARGET).elf
	@echo "ELF: $(PICO2_BUILD_DIR)/$(PICO2_TARGET).elf"
	@echo "UF2: $(PICO2_BUILD_DIR)/$(PICO2_TARGET).uf2"
endif

# Host picotool with libusb (no sudo). USB nodes still need install_picotool_udev.sh.
pico-picotool:
	src/pico/rp2350/install_picotool.sh

# Flash serial 3973D48FD625B2E8 via picotool load -f -x (BOOTSEL MSC fallback).
pico-flash: pico-kernel-rp2350
	src/pico/rp2350/flash.sh $(PICO2_BUILD_DIR)/$(PICO2_TARGET).uf2

# Sideline: Explorer ST7789 test UF2. Does not link the pico kernel / MiniCreate.
# EXPLORER_FALLBACK_PINS=1 uses CircuitPython demo DC=GP20 RESET=GP21.
pico-explorer-test:
ifeq ($(PICO2_ARM_GCC),)
	@echo "skip: pico-explorer-test: arm-none-eabi-gcc not found. Install Arch packages: $(PICO2_SKIP_PACKAGES)"
else ifeq ($(wildcard $(PICO_SDK_PATH)/pico_sdk_init.cmake),)
	@echo "skip: pico-explorer-test: PICO_SDK_PATH missing pico_sdk_init.cmake (tried $(PICO_SDK_PATH)). Install Arch packages: $(PICO2_SKIP_PACKAGES)"
else
	PATH="$(PICO2_ARM_GCC_DIR):$$PATH" PICO_SDK_PATH="$(PICO_SDK_PATH)" cmake -S $(PICO2_EXPLORER_CMAKE_DIR) -B $(PICO2_EXPLORER_BUILD_DIR) \
	  -DPICO_SDK_PATH="$(PICO_SDK_PATH)" \
	  -DPICO_BOARD=pico2 \
	  -DCMAKE_BUILD_TYPE=Release \
	  -DEXPLORER_FALLBACK_PINS=$(if $(filter 1,$(EXPLORER_FALLBACK_PINS)),ON,OFF) \
	  $(if $(wildcard $(PICO2_PICOTOOL_DIR)/picotoolConfig.cmake),-Dpicotool_DIR="$(PICO2_PICOTOOL_DIR)") \
	  $(if $(wildcard $(PICO2_PIOASM_DIR)/pioasmConfig.cmake),-Dpioasm_DIR="$(PICO2_PIOASM_DIR)")
	PATH="$(PICO2_ARM_GCC_DIR):$$PATH" cmake --build $(PICO2_EXPLORER_BUILD_DIR) --target $(PICO2_EXPLORER_TARGET) -j
	@echo "=== pico-explorer-test size (RP2350 / Pico 2; not kernel, not sm_rev_mini) ==="
	PATH="$(PICO2_ARM_GCC_DIR):$$PATH" arm-none-eabi-size $(PICO2_EXPLORER_BUILD_DIR)/$(PICO2_EXPLORER_TARGET).elf
	@echo "ELF: $(PICO2_EXPLORER_BUILD_DIR)/$(PICO2_EXPLORER_TARGET).elf"
	@echo "UF2: $(PICO2_EXPLORER_BUILD_DIR)/$(PICO2_EXPLORER_TARGET).uf2"
endif

# Sideline: not a dependency of mini / all / test. Display TUs stay off PICO_KERNEL_LIB_SRCS.
pico-move-test: $(PICO_MOVE_TEST)
	./$(PICO_MOVE_TEST)

$(PICO_MOVE_TEST): tests/test_pico_move_tileset.c src/pico/pico_oam_from_samus.c \
		src/pico/pico_frame_wire.c src/pico/pico_frame_packet.c \
		src/pico/pico_ls_assets.c src/pico/pico_ls_room.c \
		src/pico/scanline_mode1.c \
		src/pico/pico_viewport.c $(PICO_KERNEL_LIB)
	$(CC) $(PICO_CFLAGS) -Isrc/pico tests/test_pico_move_tileset.c \
		src/pico/pico_oam_from_samus.c src/pico/pico_frame_wire.c \
		src/pico/pico_frame_packet.c src/pico/pico_ls_assets.c \
		src/pico/pico_ls_room.c \
		src/pico/scanline_mode1.c src/pico/pico_viewport.c \
		-o $@ -L. -lsm_rev_pico_kernel $(PICO_LDFLAGS)

# Sideline: not a dependency of mini / all / test. Isolates packed LS layers.
pico-ls-layers-test: $(PICO_LS_LAYERS_TEST)
	mkdir -p out
	./$(PICO_LS_LAYERS_TEST)

$(PICO_LS_LAYERS_TEST): tests/test_pico_ls_layers.c src/pico/pico_ls_assets.c \
		src/pico/pico_frame_packet.c src/pico/scanline_mode1.c \
		src/pico/pico_oam_from_samus.c src/pico/pico_oam_gunship.c \
		$(PICO_KERNEL_LIB)
	$(CC) $(PICO_CFLAGS) -Isrc/pico tests/test_pico_ls_layers.c \
		src/pico/pico_ls_assets.c src/pico/pico_frame_packet.c \
		src/pico/scanline_mode1.c src/pico/pico_oam_from_samus.c \
		src/pico/pico_oam_gunship.c \
		-o $@ -L. -lsm_rev_pico_kernel $(PICO_LDFLAGS)

# Sideline: not a dependency of mini / all / test.
pico-feel-test: $(PICO_FEEL_TEST)
	./$(PICO_FEEL_TEST)

$(PICO_FEEL_TEST): tests/test_pico_feel.c $(PICO_KERNEL_LIB)
	$(CC) $(PICO_CFLAGS) tests/test_pico_feel.c -o $@ -L. -lsm_rev_pico_kernel $(PICO_LDFLAGS)

# Sideline: Explorer A/B/X/Y mask → MiniStepButtons. No GPIO, not sm_rev_mini.
pico-explorer-buttons-test: $(PICO_EXPLORER_BUTTONS_TEST)
	./$(PICO_EXPLORER_BUTTONS_TEST)

$(PICO_EXPLORER_BUTTONS_TEST): tests/test_pico_explorer_buttons.c \
		src/pico/explorer_buttons.c src/pico/pico_viewport.c $(PICO_KERNEL_LIB)
	$(CC) $(PICO_CFLAGS) -Isrc/pico tests/test_pico_explorer_buttons.c \
		src/pico/explorer_buttons.c src/pico/pico_viewport.c \
		-o $@ -L. -lsm_rev_pico_kernel $(PICO_LDFLAGS)

# Sideline: sm_rev-17t. Left/right Samus animation parity: every packed pose's
# bank 0x91 delay stream is inside the packed window, each L/R pair packs the
# same frame count, a held Left animates as much as a held Right (standing and
# morphball), and no pose mini can reach falls back to the static stand pose.
pico-samus-anim-lr-test: $(PICO_SAMUS_ANIM_LR_TEST)
	./$(PICO_SAMUS_ANIM_LR_TEST)

$(PICO_SAMUS_ANIM_LR_TEST): tests/test_pico_samus_anim_lr.c \
		src/pico/pico_oam_from_samus.c src/pico/pico_ls_room.c \
		src/pico/pico_frame_packet.c src/pico/pico_ls_assets.c \
		src/pico/scanline_mode1.c \
		src/pico/assets/pico_samus_anim.inc $(PICO_KERNEL_LIB)
	$(CC) $(PICO_CFLAGS) -Isrc/pico tests/test_pico_samus_anim_lr.c \
		src/pico/pico_oam_from_samus.c src/pico/pico_ls_room.c \
		src/pico/pico_frame_packet.c src/pico/pico_ls_assets.c \
		src/pico/scanline_mode1.c \
		-o $@ -L. -lsm_rev_pico_kernel $(PICO_LDFLAGS)

# Sideline: not a dependency of mini / all / test. sm_rev-28r morphball entry
# and exit through the real Explorer button mapper.
pico-morph-test: $(PICO_MORPH_TEST)
	./$(PICO_MORPH_TEST)

$(PICO_MORPH_TEST): tests/test_pico_morph_input.c \
		src/pico/explorer_buttons.c src/pico/pico_ls_room.c \
		src/pico/pico_ls_assets.c src/pico/pico_oam_from_samus.c \
		src/pico/pico_frame_packet.c src/pico/pico_frame_wire.c \
		$(PICO_KERNEL_LIB)
	$(CC) $(PICO_CFLAGS) -Isrc/pico tests/test_pico_morph_input.c \
		src/pico/explorer_buttons.c src/pico/pico_ls_room.c \
		src/pico/pico_ls_assets.c src/pico/pico_oam_from_samus.c \
		src/pico/pico_frame_packet.c src/pico/pico_frame_wire.c \
		-o $@ -L. -lsm_rev_pico_kernel $(PICO_LDFLAGS)

clean: clean_obj
clean_obj:
	@$(RM) $(OBJS) $(TARGET_EXEC) $(MINI_TARGET_EXEC) $(MODDABLE_TARGET_EXEC) $(MINI_KERNEL_OBJS) $(MINI_KERNEL_LIB) $(MINI_BROWSER_LIB) $(MINI_ROLLBACK_TEST) $(MINI_PREDICT_TEST) $(MINI_PREDICT_GOLDEN) $(MINI_WRAM_PEEK_TEST) $(MINI_PREDICT_CLI) $(MINI_RUST_HOST) src/embedded/*.o src/embedded/*.c $(PICO_KERNEL_LIB_OBJS) $(PICO_KERNEL_LIB) $(PICO_TARGET_EXEC) $(PICO_KERNEL_TEST) $(PICO_MOVE_TEST) $(PICO_LS_LAYERS_TEST) $(PICO_FEEL_TEST) $(PICO_EXPLORER_BUTTONS_TEST) $(PICO_SAMUS_ANIM_LR_TEST) $(PICO_MORPH_TEST) src/pico/*.pico.o
	@$(RM) -r $(PICO2_BUILD_DIR) $(PICO2_EXPLORER_BUILD_DIR)

test: all
	$(PYTHON) tests/run_tests.py -v

test-fast: all
	$(PYTHON) tests/run_tests.py --fast -v
