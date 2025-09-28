#include "button.h"
#include "ds1390.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdio.h>

static void button_1_task(void *pvParameters) {
    bool last_state = true;
    ds1390_time_t time;
    while (1) {
        bool current_state = gpio_get_level(BUTTON_1_PIN);
        if (last_state && !current_state) {
            ds1390_read_time(&time);
            printf("Button 1 pressed! Time: %02d:%02d:%02d.%02d\n",
                   time.hours, time.minutes, time.seconds, time.hundredths);
        }
        last_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void button_2_task(void *pvParameters) {
    bool last_state = true;
    uint8_t control;
    while (1) {
        bool current_state = gpio_get_level(BUTTON_2_PIN);
        if (last_state && !current_state) {
            ds1390_read_control(&control);
            printf("Button 2 pressed! Status: %02d\n", control);
        }
        last_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void button_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_1_PIN) | (1ULL << BUTTON_2_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
}

void button_start_tasks(void) {
    xTaskCreate(button_1_task, "Button 1 Task", 2048, NULL, 5, NULL);
    xTaskCreate(button_2_task, "Button 2 Task", 2048, NULL, 5, NULL);
}