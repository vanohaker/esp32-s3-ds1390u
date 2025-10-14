// main.c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include "button.h"
#include "display.h"

static const char *TAG = "MAIN";

void app_main(void) {
    // Инициализация модулей
    // init_display();
    // if (spi == NULL) {
    //     ESP_LOGE(TAG, "Display initialization failed");
    //     return;
    // }
    epaper_display_init();
    init_buttons();
    if (button_queue == NULL) {
        ESP_LOGE(TAG, "Button initialization failed");
        return; // Прекращаем, если очередь не создана
    }
    

    // Создание задач
    xTaskCreate(button_task, "button_task", 2048, NULL, 6, NULL);
    // xTaskCreate(display_task, "display_task", 4096, NULL, 5, NULL);
    xTaskCreate(epaper_display_task, "epaper_display_task", 4096, NULL, 5, NULL);
}