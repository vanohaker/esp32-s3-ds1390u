// display.h
#ifndef DISPLAY_H
#define DISPLAY_H

#include <freertos/FreeRTOS.h>
#include <driver/spi_master.h>
#include "button.h"

// Макросы для SPI
// #define SPI_HOST SPI2_HOST
// #define DMA_CHAN SPI_DMA_CH_AUTO

// Глобальные переменные
extern spi_device_handle_t spi;
extern volatile uint8_t is_redraw;

// Функции
void init_display(void);
void display_task(void *pvParameters);

void epaper_display_init(void);
void epaper_display_task(void *pvParameters);

#endif // DISPLAY_H