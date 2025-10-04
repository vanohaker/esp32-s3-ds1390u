// rtc.c
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "driver/rtc_cntl.h"
#include "rtc.h"
#include "menu.h"

void init_rtc(void) {
    struct tm default_time = {
        .tm_year = 2023 - 1900,
        .tm_mon = 0,
        .tm_mday = 1,
        .tm_hour = 0,
        .tm_min = 0,
        .tm_sec = 0
    };
    rtc_data.rtc_time = default_time;
    rtc_data.synced = false;
}

void rtc_task(void *pvParameters) {
    TickType_t last_update = xTaskGetTickCount();
    const TickType_t update_interval = pdMS_TO_TICKS(60000); // 60 секунд

    while (1) {
        if (sync_requested || (xTaskGetTickCount() - last_update >= update_interval)) {
            if (gps_data.time_valid) {
                rtc_data.rtc_time = gps_data.gps_time;
                rtc_data.synced = true;
                sync_requested = false;
                last_update = xTaskGetTickCount();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}