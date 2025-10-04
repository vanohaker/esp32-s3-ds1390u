// main.c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "display.h"

void app_main(void) {
    // Инициализация модулей
    init_display();

    // Создание задач
    xTaskCreate(display_task, "display_task", 4096, NULL, 5, NULL);
}