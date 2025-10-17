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

#define I2C_MASTER_SCL_IO           22    // GPIO pin for SCL
#define I2C_MASTER_SDA_IO           21    // GPIO pin for SDA
#define I2C_MASTER_NUM              I2C_NUM_0 // I2C port number
#define I2C_MASTER_FREQ_HZ          100000 // I2C clock frequency
#define I2C_MASTER_TX_BUF_LEN       0     // Master does not need a buffer for sending
#define I2C_MASTER_RX_BUF_LEN       0     // Master does not need a buffer for receiving
#define SLAVE_ADDR                  0x3C  // Example slave address (e.g., MPU6050)
#define SLAVE_REGISTER_ADDR         0xbc  // Example register to read from (e.g., WHO_AM_I for MPU6050)

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


uint8_t u8x8_byte_4wire_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

#endif // DISPLAY_H