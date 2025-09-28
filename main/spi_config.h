#ifndef SPI_CONFIG_H
#define SPI_CONFIG_H

#include "driver/spi_master.h"

// Пины SPI
#define PIN_NUM_CS   10
#define PIN_NUM_MOSI 11
#define PIN_NUM_CLK  12
#define PIN_NUM_MISO 13

// Инициализация SPI
void spi_init(void);

// Получение дескриптора SPI
spi_device_handle_t spi_get_handle(void);

#endif // SPI_CONFIG_H