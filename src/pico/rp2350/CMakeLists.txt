cmake_minimum_required(VERSION 3.13)

# Stock Pico 2. Must be set before pico_sdk_import / project().
set(PICO_BOARD pico2 CACHE STRING "Pico board" FORCE)

include(${CMAKE_CURRENT_LIST_DIR}/pico_sdk_import.cmake)

project(sm_rev_pico2 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

set(SM_REV_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../..")
get_filename_component(SM_REV_ROOT "${SM_REV_ROOT}" ABSOLUTE)

set(PICO_KERNEL_SRCS_CMAKE "${CMAKE_BINARY_DIR}/pico_kernel_lib_srcs.cmake")
if (NOT EXISTS "${PICO_KERNEL_SRCS_CMAKE}")
  message(FATAL_ERROR
    "${PICO_KERNEL_SRCS_CMAKE} is missing. Configure via `make pico-kernel-rp2350` "
    "so the Makefile can emit PICO_KERNEL_LIB_SRCS (do not fall back to MINI_SRCS).")
endif()
include("${PICO_KERNEL_SRCS_CMAKE}")

add_executable(sm_rev_pico2
  ${CMAKE_CURRENT_LIST_DIR}/pico2_main.c
  ${CMAKE_CURRENT_LIST_DIR}/st7789_explorer.c
  ${SM_REV_ROOT}/src/pico/explorer_buttons.c
  ${SM_REV_ROOT}/src/pico/pico_viewport.c
  ${SM_REV_ROOT}/src/pico/pico_oam_from_samus.c
  ${SM_REV_ROOT}/src/pico/pico_oam_gunship.c
  ${SM_REV_ROOT}/src/pico/pico_ls_assets.c
  ${SM_REV_ROOT}/src/pico/pico_ls_room.c
  ${SM_REV_ROOT}/src/pico/pico_frame_packet.c
  ${SM_REV_ROOT}/src/pico/scanline_mode1.c
  ${PICO_KERNEL_LIB_SRCS}
)

target_include_directories(sm_rev_pico2 PRIVATE
  ${SM_REV_ROOT}
  ${SM_REV_ROOT}/src
  ${SM_REV_ROOT}/src/mini
  ${SM_REV_ROOT}/src/pico
  ${CMAKE_CURRENT_LIST_DIR}
)
target_compile_options(sm_rev_pico2 PRIVATE
  -O2
  -fno-strict-aliasing
  -ffunction-sections
  -fdata-sections
  -Wall
  -Wno-format
  -Wno-unused-function
  -Wno-maybe-uninitialized
  -Wno-unused-variable
  -Wno-unused-but-set-variable
  -Wno-unknown-pragmas
)

# Heap 64KiB for MiniCreate. Keep default 2KiB stack: RP2350 SCRATCH_Y is 4KiB.
# USB CDC + vendor reset iface so `picotool load -f -x` can force BOOTSEL.
target_compile_definitions(sm_rev_pico2 PRIVATE
  CURRENT_BUILD=BUILD_PICO
  SYSTEM_VOLUME_MIXER_AVAILABLE=0
  PICO_HEAP_SIZE=0x10000
  PICO_STDIO_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE=1
  PICO_STDIO_USB_ENABLE_RESET_VIA_BAUD_RATE=1
)

target_link_libraries(sm_rev_pico2
  pico_stdlib
  hardware_spi
  hardware_pwm
  hardware_dma
)

pico_enable_stdio_usb(sm_rev_pico2 1)
pico_enable_stdio_uart(sm_rev_pico2 1)
pico_add_extra_outputs(sm_rev_pico2)
