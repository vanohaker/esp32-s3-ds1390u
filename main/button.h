#ifndef BUTTON_H
#define BUTTON_H

// Пины кнопок
#define BUTTON_1_PIN 1
#define BUTTON_2_PIN 2

// Инициализация кнопок
void button_init(void);

// Запуск задач FreeRTOS для кнопок
void button_start_tasks(void);

#endif // BUTTON_H