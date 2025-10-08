#include <u8g2.h>
#include <mui.h>
#include <mui_u8g2.h>
#include <esp_log.h>
#include "menu.h"

static const char *TAG = "MENU";

uint8_t mui_hrule(mui_t *ui, uint8_t msg)
{
  u8g2_t *u8g2 = mui_get_U8g2(ui);
  switch(msg)
  {
    case MUIF_MSG_DRAW:
    //   u8g2_DrawFrame(u8g2,0,0,u8g2_GetDisplayWidth(u8g2), u8g2_GetDisplayHeight(u8g2));
      u8g2_DrawHLine(u8g2, 0, mui_get_y(ui), u8g2_GetDisplayWidth(u8g2));
      break;
  }
  return 0;
}

uint8_t mui_button(mui_t *mui, uint8_t msg) {
    u8g2_t *u8g2 = mui_get_U8g2(mui);
    switch (msg) {
        case MUIF_MSG_DRAW:
            u8g2_DrawStr(u8g2, mui_get_x(mui), mui_get_y(mui), mui_get_text(mui));
            break;
        case MUIF_MSG_CURSOR_ENTER:
            u8g2_SetFont(u8g2, u8g2_font_helvB08_tr);
            break;
        case MUIF_MSG_CURSOR_LEAVE:
            u8g2_SetFont(u8g2, u8g2_font_helvR08_tr);
            break;
        case MUIF_MSG_CURSOR_SELECT:
            ESP_LOGI(TAG, "Selected: %s", mui_get_text(mui));
            return 1;
    }
    return 0;
}

// Callback для навигационных кнопок (визуальные прямоугольники)
uint8_t mui_nav_button(mui_t *mui, uint8_t msg) {
    u8g2_t *u8g2 = mui_get_U8g2(mui);
    uint8_t x = mui_get_x(mui);
    uint8_t y = mui_get_y(mui);
    const char *text = mui_get_text(mui);

    switch (msg) {
        case MUIF_MSG_DRAW: {
            // Размеры кнопки
            uint8_t w = 28;
            uint8_t h = 8;
            // Отрисовка прямоугольника
            u8g2_DrawFrame(u8g2, x, y, w, h);
            // Центрирование текста
            uint8_t text_width = u8g2_GetStrWidth(u8g2, text);
            uint8_t text_x = x + (w - text_width) / 2;
            uint8_t text_y = y + h - 1; // Базовая линия текста
            u8g2_DrawStr(u8g2, text_x, text_y, text);
            break;
        }
        case MUIF_MSG_CURSOR_ENTER:
            // Выделение кнопки (заполненный прямоугольник)
            u8g2_DrawBox(u8g2, mui_get_x(mui), mui_get_y(mui), 28, 8);
            u8g2_SetDrawColor(u8g2, 0); // Инверсия цвета для текста
            u8g2_DrawStr(u8g2, mui_get_x(mui) + (28 - u8g2_GetStrWidth(u8g2, mui_get_text(mui))) / 2, mui_get_y(mui) + 7, mui_get_text(mui));
            u8g2_SetDrawColor(u8g2, 1); // Сброс цвета
            break;
        case MUIF_MSG_CURSOR_LEAVE:
            // Отрисовка без выделения
            u8g2_DrawFrame(u8g2, mui_get_x(mui), mui_get_y(mui), 28, 8);
            u8g2_DrawStr(u8g2, mui_get_x(mui) + (28 - u8g2_GetStrWidth(u8g2, mui_get_text(mui))) / 2, mui_get_y(mui) + 7, mui_get_text(mui));
            break;
    }
    return 0;
}

// Список полей интерфейса (Field List) - это ядро вашего меню
muif_t menu_list[] = {
    MUIF_U8G2_FONT_STYLE(0, u8g2_font_courB08_tr),   // Стиль шрифта 0
    MUIF_U8G2_FONT_STYLE(1, u8g2_font_unifont_t_symbols), // Стиль для символов
    MUIF_U8G2_LABEL(),                               // Для меток (MUI_LABEL)
    MUIF_RO("HR", mui_hrule),                        // Горизонтальная линия
    MUIF_BUTTON("B1", mui_button),                   // Кнопка 1
    MUIF_BUTTON("B2", mui_button),                   // Кнопка 2
    MUIF_BUTTON("B3", mui_button),                   // Кнопка 3
    MUIF("N1", 1, 0, mui_nav_button),                      // Навигационная кнопка "Вверх"
    MUIF("N2", 1, 0, mui_nav_button),                      // Навигационная кнопка "Вниз"
    MUIF("N3", 1, 0, mui_nav_button),                      // Навигационная кнопка "Назад"
    MUIF("N4", 1, 0, mui_nav_button)                       // Навигационная кнопка "Выбор"
};

// Строки определения форм (Form Definition Strings)
fds_t menu_data[] =
MUI_FORM(0)                  // Форма с ID 1
MUI_STYLE(0)                 // Установка стиля шрифта 0
MUI_LABEL(5, 8, " Menu")     // Метка " Menu" на (5,8)
MUI_XY("HR", 0, 10)          // Горизонтальная линия на y=10
MUI_XYT("B1", 0, 20, "GPS")  // Кнопка B1 с текстом "Item 1" на (0,20)
MUI_XYT("B2", 0, 30, "Lora")  // Кнопка B2 на (0,30)
MUI_XYT("B3", 0, 40, "Settings")  // Кнопка B3 на (0,40)
MUI_XYT("N1",  2, 56, "\xe2\x8f\xb6")  // Кнопка "Вверх" (x=2, y=56)
MUI_XYT("N2", 34, 56, "\xe2\x8f\xb7")  // Кнопка "Вниз" (x=34, y=56)
MUI_XYT("N3", 66, 56, "\xe2\x8f\xb4")  // Кнопка "Назад" (x=66, y=56)
MUI_XYT("N4", 98, 56, "\xe2\x8f\xb5") // Кнопка "Выбор" (x=98, y=56)
;

size_t menu_list_size = sizeof(menu_list) / sizeof(muif_t);