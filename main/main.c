#include "spi_config.h"
#include "button.h"
#include "ds1390.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {
    // Инициализация подсистем
    spi_init();
    button_init();

    ds1390_enable_trickle_charger();
    ds1390_enable_oscillator();

    // Установка начального времени (например, 12:34:56.78)
    ds1390_time_t init_time = {
        .hundredths = 78,
        .seconds = 56,
        .minutes = 34,
        .hours = 12
    };
    ds1390_write_time(&init_time);

    // Проверка начального времени
    ds1390_time_t check_time;
    ds1390_read_time(&check_time);
    printf("Initial time set: %02d:%02d:%02d.%02d\n",
           check_time.hours, check_time.minutes, check_time.seconds, check_time.hundredths);
    
    // Запуск задач
    button_start_tasks();
}