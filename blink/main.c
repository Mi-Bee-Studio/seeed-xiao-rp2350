/*
 * blink — seeed-xiao-rp2350 基线工程（测试固件）。
 *
 * 板载 LED（GP25）每秒翻转；每 10 秒一条心跳日志（uptime/heap），供
 * serialtap 持续采集验证。同构于 esp32-s3-zero/blink 与 luatos-esp32c3/blink。
 *
 * 看门狗（全家桶基线强制）：RP2040/RP2350 无 TWDT，用片上硬件看门狗——
 * 8s 超时触发重启；主循环每拍 watchdog_update() 喂狗。
 * 本板无网络，Web/API OTA 基线属硬件豁免（UF2/BOOTSEL/picotool 为升级路径）。
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "hardware/gpio.h"
#include "pico/unique_id.h"

/* pico-sdk 官方板头 seeed_xiao_rp2350.h 的定义，不硬编码 */
#ifndef PICO_DEFAULT_LED_PIN
#define PICO_DEFAULT_LED_PIN 25
#endif

int main(void)
{
    stdio_init_all();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    char id[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];
    pico_get_unique_board_id_string(id, sizeof(id));

    printf("\nblink ready: led=GP%d board-id=%s flash=%uK\n",
           PICO_DEFAULT_LED_PIN, id,
           (unsigned)(PICO_FLASH_SIZE_BYTES / 1024));

    /* 硬件看门狗：8s 超时（约 8 个主循环周期喂不上即重启自恢复） */
    watchdog_enable(8000, 1);

    absolute_time_t next_heartbeat = make_timeout_time_ms(0);
    int beat = 0;
    bool on = false;
    while (true) {
        watchdog_update();

        on = !on;
        /* USER LED 为低电平点亮（active-low，见 Seeed wiki 引脚表） */
        gpio_put(PICO_DEFAULT_LED_PIN, !on);
        sleep_ms(1000);

        if (absolute_time_diff_us(next_heartbeat, get_absolute_time()) >= 0) {
            beat = (beat + 1) % 10;
            next_heartbeat = make_timeout_time_ms(10000);
            printf("heartbeat uptime=%llds\n",
                   (long long)(to_ms_since_boot(get_absolute_time()) / 1000));
        }
    }
    return 0;
}
