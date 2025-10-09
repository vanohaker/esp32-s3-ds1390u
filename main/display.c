// display.c
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <u8g2.h>
#include <mui.h>
#include <mui_u8g2.h>
#include <esp_log.h>
#include "display.h"
#include "button.h"
#include "menu.h"

static const char* TAG = "DISPLAY";

// Глобальные переменные
u8g2_t u8g2;
mui_t mui;
spi_device_handle_t spi;
volatile uint8_t is_redraw = 1;

// https://github.com/olikraus/u8g2/wiki/Porting-to-new-MCU-platform#communication-callback-eg-u8x8_byte_hw_i2c
uint8_t u8x8_byte_4wire_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    esp_err_t ret;

    switch (msg) {
        case U8X8_MSG_BYTE_SEND:
            spi_transaction_t t = {
                .length = 8 * arg_int,
                .tx_buffer = arg_ptr,
            };
            ret = spi_device_transmit(spi, &t);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "SPI transmit error: %s", esp_err_to_name(ret));
                return 0;
            }
            break;
        case U8X8_MSG_BYTE_INIT:
            break;
        case U8X8_MSG_BYTE_SET_DC:
            gpio_set_level(PIN_DC, arg_int);
            break;
        case U8X8_MSG_BYTE_START_TRANSFER:
            gpio_set_level(PIN_CS, 0);
            break;
        case U8X8_MSG_BYTE_END_TRANSFER:
            gpio_set_level(PIN_CS, 1);
            break;
        default:
            return 0;
    }
    return 1;
}

// Функция для управления GPIO (DC, RST) и задержками
// https://github.com/olikraus/u8g2/wiki/Porting-to-new-MCU-platform#the-uc-specific-gpio-and-delay-callback
uint8_t u8x8_gpio_and_delay_esp32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    esp_err_t ret;
    switch (msg) {
        case U8X8_MSG_GPIO_AND_DELAY_INIT: // Вызывается один раз во время фазы инициализации и может использоваться для настройки пинов
            gpio_config_t io_conf = {
                .pin_bit_mask = (1ULL << PIN_CS) | (1ULL << PIN_DC) | (1ULL << PIN_RESET),
                .mode = GPIO_MODE_OUTPUT,
                .pull_up_en = GPIO_PULLUP_DISABLE,
                .pull_down_en = GPIO_PULLDOWN_ENABLE,
                .intr_type = GPIO_INTR_POSEDGE
            };
            ret = gpio_config(&io_conf);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "GPIO display pin config failed in INIT: %s\n", esp_err_to_name(ret));
                return 0;
            } 
            ESP_LOGI(TAG, "GPIO display pin INIT succeeded.");
            // Установка начальных уровней
            gpio_set_level(PIN_CS, 1);
            gpio_set_level(PIN_DC, 1);
            gpio_set_level(PIN_RESET, 1);
            // Сброс дисплея
            gpio_set_level(PIN_RESET, 0);
            vTaskDelay(1 / portTICK_PERIOD_MS);
            gpio_set_level(PIN_RESET, 1);
            vTaskDelay(5 / portTICK_PERIOD_MS);
            spi_bus_config_t buscfg = {
                .mosi_io_num = PIN_MOSI,
                .miso_io_num = -1,
                .sclk_io_num = PIN_SCLK,
                .quadwp_io_num = -1,
                .quadhd_io_num = -1,
                .max_transfer_sz = 4096,
            };
            ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "SPI bus init failed: %s\n", esp_err_to_name(ret));
                return 0;
            }
            spi_device_interface_config_t devcfg = {
                .clock_speed_hz = 8000000,
                .mode = 0,
                .spics_io_num = -1,
                .queue_size = 7,
            };
            ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "SPI device add failed: %s\n", esp_err_to_name(ret));
                return 0;
            }
            ESP_LOGI(TAG, "GPIO, buttons and SPI initialized in INIT\n");
            break;
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
            gpio_set_level(PIN_DC, arg_int);
            break;
        case U8X8_MSG_GPIO_RESET:
            gpio_set_level(PIN_RESET, arg_int);
            break;
        case U8X8_MSG_GPIO_CS:
            gpio_set_level(PIN_CS, arg_int);
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
    mui_Init(&mui, &u8g2, menu_data, menu_list, menu_list_size);
    mui_GotoForm(&mui, 0, 0);
    ESP_LOGI(TAG, "Display init complete.");
}


void display_task(void *pvParameters) {
    button_event_t event;
    while (1) {
        if (xQueueReceive(button_queue, &event, 0)) {
            ESP_LOGI(TAG, "%02X", event);
            switch (event) {
                case BUTTON_EVENT_UP_PRESS:
                    mui_PrevField(&mui);
                    is_redraw = 1;
                    break;
                case BUTTON_EVENT_DOWN_PRESS:
                    mui_NextField(&mui);
                    is_redraw = 1;
                    break;
                case BUTTON_EVENT_SELECT_PRESS:
                    mui_SendSelect(&mui);
                    is_redraw = 1;
                    break;
                case BUTTON_EVENT_BACK_PRESS:
                    mui_GotoForm(&mui, 1, 0);  // Возврат к началу формы
                    is_redraw = 1;
                    break;
                default:
                    break;
            }
        }
        if (is_redraw) {
            // Очистка буфера
            u8g2_ClearBuffer(&u8g2);
            // Рендеринг
            mui_Draw(&mui);
            // Отправка на дисплей
            u8g2_SendBuffer(&u8g2);
            is_redraw = 0;
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);  // Обновление каждую секунду
    }
}