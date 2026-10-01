# blink — Seeed XIAO RP2350 基线工程（pico-sdk）
cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(blink C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(blink
    main.c
)

# 板级事实以 pico-sdk 官方板头为准：seeed_xiao_rp2350
# （LED=GP25 · WS2812=GP22+电源 GP23 · 2MB flash · RP2350A）
target_link_libraries(blink
    pico_stdlib
    hardware_watchdog
)

# 串口走 USB CDC（本板无 USB-UART 桥；BOOTSEL 模式下是 U 盘不是串口；
# pico-sdk 默认支持 1200bps 触碰软重启进 BOOTSEL，免按 BOOT 键）
pico_enable_stdio_usb(blink 1)
pico_enable_stdio_uart(blink 0)

pico_add_extra_outputs(blink)   # .uf2 / .elf / .bin / .hex / .dis
