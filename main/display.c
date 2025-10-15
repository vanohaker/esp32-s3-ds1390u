// display.c
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include "epaper-29-dke.h"
#include "esp_random.h"

#ifndef EPD_BLACK
#define EPD_BLACK 1
#endif

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

#define PIN_MOSI   13
#define PIN_SCLK   14
#define PIN_CS     15
#define PIN_DC     16
#define PIN_RESET  17
#define PIN_BUSY   18

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
    ESP_LOGI("EPAPER_INIT", "start");
    epaper_conf_t conf = {0};
    ESP_LOGI("EPAPER_INIT", "conf struct ok");
    conf.mosi_pin = PIN_MOSI;
    conf.sck_pin = PIN_SCLK;
    conf.cs_pin  = PIN_CS;
    conf.dc_pin  = PIN_DC;
    conf.reset_pin = PIN_RESET;
    conf.busy_pin  = PIN_BUSY;
    conf.clk_freq_hz = 2000000; // 2 MHz
    conf.spi_host = SPI2_HOST;
    conf.width  = EPD_WIDTH;
    conf.height = EPD_HEIGHT;
    conf.dc_lev_data = 1;
    conf.dc_lev_cmd  = 0;
    conf.rst_active_level  = 0;
    conf.busy_active_level = 0;
    conf.color_inv = 0;
    conf.dma_chan = 0;
    ESP_LOGI("EPAPER_INIT", "conf filled");

    epd = iot_epaper_create(NULL, &conf);
    ESP_LOGI("EPAPER_INIT", "iot_epaper_create done");
    if (epd == NULL) {
        ESP_LOGE("EPAPER_INIT", "E-paper device create failed");
        return;
    }
    iot_epaper_clean_paint(epd, 0);
    ESP_LOGI("EPAPER_INIT", "clean_paint done");
    iot_epaper_display_frame(epd, NULL);
    ESP_LOGI("EPAPER_INIT", "display_frame done");
    ESP_LOGI("EPAPER_INIT", "init complete");
}

// простая перерисовка через epaper
void epaper_redraw_screen(void) {
    if (epd == NULL) {
        ESP_LOGW(TAG, "E-paper not initialized");
        return;
    }
    ESP_LOGI("EPAPER_TASK", "Frame sent");
    iot_epaper_clean_paint(epd, 0);
    iot_epaper_display_frame(epd, NULL);
}

// небольшой 3x5 шрифтовый массив для цифр 0..9
static const uint8_t digit_font[10][5] = {
    {7,5,5,5,7}, //0
    {2,6,2,2,7}, //1
    {7,1,7,4,7}, //2
    {7,1,7,1,7}, //3
    {5,5,7,1,1}, //4
    {7,4,7,1,7}, //5
    {7,4,7,5,7}, //6
    {7,1,1,1,1}, //7
    {7,5,7,5,7}, //8
    {7,5,7,1,7}  //9
};

// нарисовать одну цифру (3x5) как набор заполненных прямоугольников
static void draw_digit(epaper_handle_t dev, int digit, int x, int y, int dot_size, int spacing, int color)
{
    if (digit < 0 || digit > 9) return;
    for (int row = 0; row < 5; ++row) {
        uint8_t bits = digit_font[digit][row];
        for (int col = 0; col < 3; ++col) {
            if (bits & (1 << (2 - col))) {
                int px = x + col * (dot_size + spacing);
                int py = y + row * (dot_size + spacing);
                iot_epaper_draw_filled_rectangle(dev, px, py, px + dot_size - 1, py + dot_size - 1, color);
            }
        }
    }
}

// нарисовать целое число, центрируя по ширине экрана
static void draw_number(epaper_handle_t dev, uint32_t num, int y, int dot_size, int spacing, int gap, int color)
{
    int digits[12];
    int nd = 0;
    if (num == 0) {
        digits[nd++] = 0;
    } else {
        while (num && nd < 12) {
            digits[nd++] = num % 10;
            num /= 10;
        }
    }
    int digit_width = 3 * dot_size + 2 * spacing;
    int total_width = nd * digit_width + (nd - 1) * gap;
    int start_x = (EPD_WIDTH - total_width) / 2;
    for (int i = 0; i < nd; ++i) {
        int d = digits[nd - 1 - i];
        int x = start_x + i * (digit_width + gap);
        draw_digit(dev, d, x, y, dot_size, spacing, color);
    }
}

// нарисовать одну случайную "точку" (маленький заполненный прямоугольник)
static void draw_random_dot(epaper_handle_t dev, int size, int color)
{
    uint32_t r = esp_random();
    int maxx = EPD_WIDTH - size;
    int maxy = EPD_HEIGHT - size;
    if (maxx < 0) maxx = 0;
    if (maxy < 0) maxy = 0;
    int x = (int)(r % (maxx + 1));
    int y = (int)((r >> 16) % (maxy + 1));
    iot_epaper_draw_filled_rectangle(dev, x, y, x + size - 1, y + size - 1, color);
}

// заменяем задачу обновления epaper: раз в секунду рисуем счётчик + случайную точку
void epaper_display_task(void *pvParameters) {
    int color = 1; // 1 — белый, 0 — чёрный
    while (1) {
        if (epd == NULL) {
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }
        iot_epaper_clean_paint(epd, color);
        iot_epaper_display_frame(epd, NULL);
        ESP_LOGI("EPAPER_TASK", "Frame sent (%s)", color ? "white" : "black");
        color = !color; // чередуем цвет
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