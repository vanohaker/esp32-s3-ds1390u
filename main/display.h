// display.h
#ifndef DISPLAY_H
#define DISPLAY_H

#include <freertos/FreeRTOS.h>
#include <u8g2.h>
#include <mui.h>
#include <mui_u8g2.h>
#include <driver/spi_master.h>
#include "button.h"

// Пины для дисплея SSD1309 (SPI)
#define PIN_MOSI GPIO_NUM_11
#define PIN_SCLK GPIO_NUM_12
#define PIN_CS GPIO_NUM_10
#define PIN_DC GPIO_NUM_9
#define PIN_RESET GPIO_NUM_8

// Макросы для SPI
// #define SPI_HOST SPI2_HOST
// #define DMA_CHAN SPI_DMA_CH_AUTO

// Глобальные переменные
extern u8g2_t u8g2;
extern mui_t mui;
extern spi_device_handle_t spi;
extern volatile uint8_t is_redraw;

// Функции
void init_display(void);
void display_task(void *pvParameters);
void set_contrast(uint8_t value);

#endif // DISPLAY_H