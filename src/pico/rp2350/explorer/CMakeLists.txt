cmake_minimum_required(VERSION 3.13)

# Stock Pico 2. Must be set before pico_sdk_import / project().
set(PICO_BOARD pico2 CACHE STRING "Pico board" FORCE)

include(${CMAKE_CURRENT_LIST_DIR}/../pico_sdk_import.cmake)

project(sm_rev_pico_explorer_test C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(sm_rev_pico_explorer_test
  ${CMAKE_CURRENT_LIST_DIR}/../explorer_st7789_test.c
)

target_compile_options(sm_rev_pico_explorer_test PRIVATE
  -O2
  -Wall
)

option(EXPLORER_FALLBACK_PINS "CircuitPython demo DC=GP20 RESET=GP21" OFF)
if (EXPLORER_FALLBACK_PINS)
  target_compile_definitions(sm_rev_pico_explorer_test PRIVATE EXPLORER_FALLBACK_PINS=1)
endif()

target_compile_definitions(sm_rev_pico_explorer_test PRIVATE
  PICO_STDIO_USB_CONNECT_WAIT_MS=2000
)

target_link_libraries(sm_rev_pico_explorer_test
  pico_stdlib
  hardware_spi
)

pico_enable_stdio_usb(sm_rev_pico_explorer_test 1)
pico_enable_stdio_uart(sm_rev_pico_explorer_test 0)
pico_add_extra_outputs(sm_rev_pico_explorer_test)
