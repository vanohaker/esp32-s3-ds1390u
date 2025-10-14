// display.c
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include "epaper-29-dke.h"

#include <esp_log.h>
#include <stdint.h>

#if defined(__has_include)
#  if __has_include("esp_rom/esp_rom.h")
#    include "esp_rom/esp_rom.h"
#  elif __has_include("esp_rom.h")
#    include "esp_rom.h"
#  elif __has_include("esp_rom/esp_rom_sys.h")
#    include "esp_rom/esp_rom_sys.h"
#  elif __has_include("esp_rom/ets_sys.h")
#    include "esp_rom/ets_sys.h"
#  else
extern void esp_rom_delay_us(uint32_t us);
#  endif
#else
extern void esp_rom_delay_us(uint32_t us);
#endif

#include "display.h"
#include "button.h"

#define PIN_MOSI   11
#define PIN_SCLK   12
#define PIN_CS     10
#define PIN_DC     9
#define PIN_RESET  8
#define PIN_BUSY   7

static const char* TAG = "DISPLAY";

// Глобальные переменные (убраны u8g2/mui; используем epaper)
spi_device_handle_t spi;
volatile uint8_t is_redraw = 1;
static epaper_handle_t epd = NULL;

// инициализация дисплея — теперь просто перенаправляем на epaper
void init_display(void) {
    epaper_display_init();
}

// замените реализацию инициализации e-paper на вызов iot_epaper_create
void epaper_display_init(void) {
    epaper_conf_t conf = {0};
    conf.mosi_pin = PIN_MOSI;
    conf.sck_pin = PIN_SCLK;
    conf.cs_pin  = PIN_CS;
    conf.dc_pin  = PIN_DC;
    conf.reset_pin = PIN_RESET;
    conf.busy_pin  = PIN_BUSY;
    conf.clk_freq_hz = 10000000; // 10 MHz, при необходимости снизьте
    conf.spi_host = SPI2_HOST;   // если нужно, поменяйте на другой SPI_HOST
    conf.width  = EPD_WIDTH;
    conf.height = EPD_HEIGHT;
    conf.dc_lev_data = 1;
    conf.dc_lev_cmd  = 0;
    conf.rst_active_level  = 0;
    conf.busy_active_level = 0;
    conf.color_inv = 0;

    epd = iot_epaper_create(NULL, &conf);
    if (epd == NULL) {
        ESP_LOGE(TAG, "E-paper device create failed");
        return;
    }
    iot_epaper_clean_paint(epd, 0);
    iot_epaper_display_frame(epd, NULL);
    ESP_LOGI(TAG, "E-paper display init complete.");
}

// простая перерисовка через epaper
void epaper_redraw_screen(void) {
    if (epd == NULL) {
        ESP_LOGW(TAG, "E-paper not initialized");
        return;
    }
    iot_epaper_clean_paint(epd, 0);
    iot_epaper_display_frame(epd, NULL);
}

void epaper_display_task(void *pvParameters) {
    while (1) {
        epaper_redraw_screen();
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// простой display_task — обрабатывает кнопки и вызывает перерисовку epaper
void display_task(void *pvParameters) {
    button_event_t event;
    while (1) {
        if (xQueueReceive(button_queue, &event, 0)) {
            // Пока — просто помечаем перерисовку при любом событии кнопки.
            // Здесь позже нужно будет портировать навигацию меню на ваше epaper-API.
            is_redraw = 1;
        }
        if (is_redraw) {
            epaper_redraw_screen();
            is_redraw = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}