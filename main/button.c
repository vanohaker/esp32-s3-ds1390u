#include "freertos/FreeRTOS.h"
#include <freertos/queue.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "button.h"

static const char *TAG = "BUTTON";

// Глобальная очередь
QueueHandle_t button_queue;

// Функция инициализации кнопок
void init_buttons(void) {
    esp_err_t ret;
    // Инициализация очереди для событий кнопок
    button_queue = xQueueCreate(10, sizeof(button_event_t));
    if (button_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create button queue");
        return;
    }

    // Инициализация пинов кнопок с прерываниями
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_UP) | (1ULL << BUTTON_DOWN) | (1ULL << BUTTON_SELECT) | (1ULL << BUTTON_BACK),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO config error: %s", esp_err_to_name(ret));
        button_queue = NULL;
        return;
    }
}

// Задача обработки кнопок
void button_task(void *pvParameters) {
    static bool button_state[4] = { // Текущее состояние кнопок (pressed = true)
        false, //   BUTTON_UP
        false, //   BUTTON_DOWN
        false, //   BUTTON_SELECT
        false  //   BUTTON_BACK
    };
    static bool last_state[4] = {false, false, false, false};   // Предыдущее состояние.
    static TickType_t last_change_time[4] = {0, 0, 0, 0};       // Время последнего изменения состояния, в тиках! таймера с момента начала работы freertos
    const TickType_t debounce_ms = 50 / portTICK_PERIOD_MS;

    while (1) {
        TickType_t current_time = xTaskGetTickCount();
        bool current_pressed[4] = { // Текущие стостояния пинов в момент прохода таски
            (gpio_get_level(BUTTON_DOWN) == 0), // Если кпопка нажата то true
            (gpio_get_level(BUTTON_UP) == 0),
            (gpio_get_level(BUTTON_SELECT) == 0),
            (gpio_get_level(BUTTON_BACK) == 0)
        };
        for (uint8_t index = 0; index < 4; index++) { // Перебераем массив и изем нажатую кнопку
            if (current_pressed[index] != last_state[index]) { // Если кнопка нажата или отпустилась. Текущее состояние не совпадает с прошлым.
                last_change_time[index] = current_time; // Фиксируем время нажатия на кнопку
                last_state[index] = current_pressed[index]; // Фиксируем текущее состояние 
            } else if ((current_time - last_change_time[index]) > debounce_ms) { // Если с момента нажатия кнопки прошло времени больше чем время дебаунс
                // Этот код выполнится только если прошло время больше чем время дебаунс и кнопка всё ещё нажата.
                // За время debounce_ms кнопка принимает стабильное состояние
                if (current_pressed[index] != button_state[index]) { // Если текущее состояние кнопки не такое как состояние по умолчанию
                    button_state[index] = current_pressed[index]; // Пишем в состояние кнопок, значение текущего состояния
                    if (button_state[index]) {
                        // Текущее состояние = кнопка нажата
                        button_event_t event = (button_event_t)index; // Пишем в евент номер нашей кнопки
                        xQueueSend(button_queue, &event, portMAX_DELAY);
                        ESP_LOGD(TAG, "Button %d pressed", index);
                    } else {
                        // Отпускание кнопки
                        ESP_LOGD(TAG, "Button %d released", index);
                        // Здесь можно добавить логику для отпускания, если нужно (пока не используется)
                    }
                }
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); // Частота опроса ~100 Гц
    }
}