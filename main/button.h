// buttoons.h
#ifndef BUTTONS_H
#define BUTTONS_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// Кнопки
#define BUTTON_DOWN     GPIO_NUM_1   // Вних
#define BUTTON_UP       GPIO_NUM_2   // Вверх
#define BUTTON_SELECT   GPIO_NUM_3   // Выбор
#define BUTTON_BACK     GPIO_NUM_4   // Назад

// Тип для событий кнопок
typedef enum {
    BUTTON_EVENT_UP,
    BUTTON_EVENT_DOWN,
    BUTTON_EVENT_SELECT,
    BUTTON_EVENT_BACK
} button_event_t;

// Глобальная очередь для событий кнопок
extern QueueHandle_t button_queue;

// Функции кнопок
void init_buttons(void);
void button_task(void *pvParameters);

#endif // buttons_H