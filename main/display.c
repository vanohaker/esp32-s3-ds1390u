// display.c
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "u8g2.h"
#include "u8x8.h"
#include "display.h"
#include "esp_log.h"

// Конфигурация пинов
#define OLED_RST  8
#define OLED_DC   9
#define OLED_CS   10
#define OLED_MOSI 11
#define OLED_SCK  12
#define SPI_HOST SPI2_HOST
#define DMA_CHAN SPI_DMA_CH_AUTO

// Кнопки для MUI
#define BUTTON_PREV_GPIO 1   // Up/Prev
#define BUTTON_NEXT_GPIO 2   // Down/Next
#define BUTTON_HOME_GPIO 3   // Left/Home/Back
#define BUTTON_SELECT_GPIO 4 // Right/Select

// U8g2 объект
static u8g2_t u8g2;
// SPI устройство
static spi_device_handle_t spi;
// Тег логирования
static const char* TAG = "DISPLAY";

// https://github.com/olikraus/u8g2/wiki/Porting-to-new-MCU-platform#communication-callback-eg-u8x8_byte_hw_i2c
uint8_t u8x8_byte_4wire_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch (msg) {
        case U8X8_MSG_BYTE_SEND:
            {
                spi_transaction_t t = {
                    .length = 8 * arg_int,
                    .tx_buffer = arg_ptr,
                };
                esp_err_t ret = spi_device_transmit(spi, &t);
                return ret == ESP_OK ? 1 : 0;
            }
            break;
        case U8X8_MSG_BYTE_INIT:
            break;
        case U8X8_MSG_BYTE_SET_DC:
            gpio_set_level(OLED_DC, arg_int);
            break;
        case U8X8_MSG_BYTE_START_TRANSFER:
            gpio_set_level(OLED_CS, 0);
            break;
        case U8X8_MSG_BYTE_END_TRANSFER:
            gpio_set_level(OLED_CS, 1);
            break;
        default:
            return 0;
    }
    return 1;
}

// Функция для управления GPIO (DC, RST) и задержками
// https://github.com/olikraus/u8g2/wiki/Porting-to-new-MCU-platform#the-uc-specific-gpio-and-delay-callback
uint8_t u8x8_gpio_and_delay_esp32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch (msg) {
        case U8X8_MSG_GPIO_AND_DELAY_INIT: // Вызывается один раз во время фазы инициализации и может использоваться для настройки пинов
            gpio_config_t io_conf = {
                .pin_bit_mask = (1ULL << OLED_CS) | (1ULL << OLED_DC) | (1ULL << OLED_RST),
                .mode = GPIO_MODE_OUTPUT,
                .pull_up_en = GPIO_PULLUP_DISABLE,
                .pull_down_en = GPIO_PULLDOWN_ENABLE,
                .intr_type = GPIO_INTR_POSEDGE
            };
            esp_err_t ret = gpio_config(&io_conf);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "GPIO display pin config failed in INIT: %s\n", esp_err_to_name(ret));
                return 0;
            } 
            ESP_LOGI(TAG, "GPIO display pin INIT succeeded.");
            // Установка начальных уровней
            gpio_set_level(OLED_CS, 1);
            gpio_set_level(OLED_DC, 1);
            gpio_set_level(OLED_RST, 1);
            // Сброс дисплея
            gpio_set_level(OLED_RST, 0);
            vTaskDelay(1 / portTICK_PERIOD_MS);
            gpio_set_level(OLED_RST, 1);
            vTaskDelay(5 / portTICK_PERIOD_MS);
            // Инициализация кнопок для MUI (групповая)
            gpio_config_t io_conf_btn = {
                .pin_bit_mask = (1ULL << BUTTON_PREV_GPIO) | (1ULL << BUTTON_NEXT_GPIO) |
                                (1ULL << BUTTON_HOME_GPIO) | (1ULL << BUTTON_SELECT_GPIO),
                .mode = GPIO_MODE_INPUT,
                .pull_up_en = GPIO_PULLUP_ENABLE,  // Подтяжка up (нажатие = low = 0)
                .pull_down_en = GPIO_PULLDOWN_DISABLE,
                .intr_type = GPIO_INTR_DISABLE
            };
            ret = gpio_config(&io_conf_btn);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "GPIO control buttons config failed in INIT: %s\n", esp_err_to_name(ret));
                return 0;
            }
            ESP_LOGI(TAG, "GPIO control buttons INIT succeeded.");
            spi_bus_config_t buscfg = {
                .mosi_io_num = OLED_MOSI,
                .miso_io_num = -1,
                .sclk_io_num = OLED_SCK,
                .quadwp_io_num = -1,
                .quadhd_io_num = -1,
                .max_transfer_sz = 4096,
            };
            ret = spi_bus_initialize(SPI_HOST, &buscfg, DMA_CHAN);
            if (ret != ESP_OK) {
                printf("SPI bus init failed: %s\n", esp_err_to_name(ret));
                return 0;
            }
            ESP_LOGI(TAG, "SPI bus INIT succeeded.");
            spi_device_interface_config_t devcfg = {
                .clock_speed_hz = 8000000,
                .mode = 0,
                .spics_io_num = -1,
                .queue_size = 7,
            };
            ret = spi_bus_add_device(SPI_HOST, &devcfg, &spi);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "SPI device add failed: %s\n", esp_err_to_name(ret));
                return 0;
            }
            ESP_LOGI(TAG, "SPI device INIT succeeded.");
            ESP_LOGI(TAG, "GPIO, buttons and SPI initialized in INIT\n");
            break;
        case U8X8_MSG_GPIO_MENU_SELECT:
            return gpio_get_level(BUTTON_SELECT_GPIO) ? 1 : 0;
        case U8X8_MSG_GPIO_MENU_NEXT:
            return gpio_get_level(BUTTON_NEXT_GPIO) ? 1 : 0;
        case U8X8_MSG_GPIO_MENU_PREV:
            return gpio_get_level(BUTTON_PREV_GPIO) ? 1 : 0;
        case U8X8_MSG_GPIO_MENU_HOME:
            return gpio_get_level(BUTTON_HOME_GPIO) ? 1 : 0;
        case U8X8_MSG_DELAY_NANO: // задержка arg_int * 1 наносекунда
            esp_rom_delay_us(arg_int / 1000.0);
            break;
        case U8X8_MSG_DELAY_100NANO: // задержка arg_int * 100 наносекунд
            esp_rom_delay_us(arg_int * 0.1);
            break;
        case U8X8_MSG_DELAY_10MICRO: // задержка arg_int * 10 микросекунд
            esp_rom_delay_us(arg_int * 10);
            break;
        case U8X8_MSG_DELAY_MILLI: // задержка arg_int * 1 миллисекунда
            vTaskDelay(arg_int / portTICK_PERIOD_MS);
            break;
        case U8X8_MSG_GPIO_DC:
            gpio_set_level(OLED_DC, arg_int);
            break;
        case U8X8_MSG_GPIO_RESET:
            gpio_set_level(OLED_RST, arg_int);
            break;
        case U8X8_MSG_GPIO_CS:
            gpio_set_level(OLED_CS, arg_int);
            break;
        default:
            return 0; // Неизвестное сообщение
    }
    return 1;
}

void init_display(void) {
    // Настройка U8g2 (C API, без SetPin)
    u8g2_Setup_ssd1309_128x64_noname0_f(&u8g2, U8G2_R0, u8x8_byte_4wire_hw_spi, u8x8_gpio_and_delay_esp32);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
    u8g2_ClearDisplay(&u8g2);
    printf("Display init complete\n");
}


void display_task(void *pvParameters) {
    while (1) {
        // Очистка буфера
        u8g2_ClearBuffer(&u8g2);

        // Рисование круга (центр 64,64, радиус 30)
        u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
        u8g2_DrawStr(&u8g2, 0, 10, "Circle Test");
        u8g2_DrawCircle(&u8g2, 64, 32, 20, U8G2_DRAW_ALL);  // Круг (x, y, r, all)

        // Отправка на дисплей
        u8g2_SendBuffer(&u8g2);

        vTaskDelay(pdMS_TO_TICKS(1000));  // Обновление каждую секунду
    }
}