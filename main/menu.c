#include <u8g2.h>
#include <mui.h>
#include <mui_u8g2.h>
#include <esp_log.h>
#include "menu.h"

static const char *TAG = "MENU";

uint8_t mui_hrline(mui_t *ui, uint8_t msg)
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

// Список полей интерфейса (Field List) - это ядро вашего меню
muif_t menu_list[] = {
    MUIF_U8G2_FONT_STYLE(0, u8g2_font_courB08_tr),   // Стиль шрифта 0
    MUIF_U8G2_FONT_STYLE(1, u8g2_font_unifont_t_symbols), // Стиль для символов
    MUIF_U8G2_LABEL(),                               // Для меток (MUI_LABEL)
    MUIF_RO("HR", mui_hrline),                        // Горизонтальная линия
    MUIF_BUTTON("B1", mui_button),                   // Кнопка 1
    MUIF_BUTTON("B2", mui_button),                   // Кнопка 2
    MUIF_BUTTON("B3", mui_button),                   // Кнопка 3
};

// Строки определения форм (Form Definition Strings)
fds_t menu_data[] =
MUI_FORM(0)                  // Форма с ID 0
MUI_STYLE(0)                 // Установка стиля шрифта 0
MUI_LABEL(5, 8, "Menu")     // Метка " Menu" на (5,8)
MUI_XY("HR", 0, 10)          // Горизонтальная линия на y=10
MUI_XYT("B1", 0, 20, "GPS")  // Кнопка B1 с текстом "Item 1" на (0,20)
MUI_XYT("B2", 0, 30, "Lora")  // Кнопка B2 на (0,30)
MUI_XYT("B3", 0, 40, "Settings")  // Кнопка B3 на (0,40)
;

size_t menu_list_size = sizeof(menu_list) / sizeof(muif_t);