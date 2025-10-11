#include <u8g2.h>
#include <mui.h>
#include <mui_u8g2.h>
#include <esp_log.h>
#include "menu.h"

static const char *TAG = "MENU";

uint16_t list_selection1 = 0;
uint8_t brightness = 50;

uint16_t menu_get_cnt(void *data)
{
  return 2;    /* number of menu entries */
}

const char *menu_get_str(void *data, uint16_t index)
{
  static const char *settings[] = 
  { 
    MUI_0 "Back",
    MUI_35 "Brightness"
  };
  return settings[index];
}

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
            u8g2_SetFont(u8g2, u8g2_font_threepix_tr);
            break;
        case MUIF_MSG_CURSOR_LEAVE:
            u8g2_SetFont(u8g2, u8g2_font_u8glib_4_tf);
            break;
        case MUIF_MSG_CURSOR_SELECT:
            return 1;
    }
    return 0;
}

// Callback для центрированной метки
uint8_t mui_centered_label(mui_t *mui, uint8_t msg) {
    u8g2_t *ui = mui_get_u8g2(mui);
    switch (msg) {
        case MUIF_MSG_DRAW: {
            const char *text = mui_get_text(mui);
            uint8_t width = u8g2_GetStrWidth(ui, text);
            uint8_t x = (u8g2_GetDisplayWidth(ui) - width) / 2;
            u8g2_DrawStr(ui, x, mui_get_y(mui), text);
            break;
        }
    }
    return 0;
}

// Callback для Bar Graph регулировки яркости
uint8_t mui_brightness_bar(mui_t *mui, uint8_t msg) {
    u8g2_t *ui = mui_get_u8g2(mui);
    uint8_t x = mui_get_x(mui);
    uint8_t y = mui_get_y(mui);

    switch (msg) {
        case MUIF_MSG_DRAW: {
            // Рисуем Bar Graph
            uint8_t bar_width = 100;
            uint8_t bar_height = 8;
            uint8_t filled_width = (temp_brightness * bar_width) / 20; // Масштабируем 1-20 в 0-100 пикселей
            u8g2_DrawFrame(ui, x, y, bar_width, bar_height);
            u8g2_DrawBox(ui, x, y, filled_width, bar_height);
            // Рисуем значение яркости
            char buffer[16];
            snprintf(buffer, sizeof(buffer), "Brightness: %d", temp_brightness);
            uint8_t text_width = u8g2_GetStrWidth(ui, buffer);
            uint8_t text_x = (u8g2_GetDisplayWidth(ui) - text_width) / 2;
            u8g2_DrawStr(ui, text_x, y - 2, buffer);
            break;
        }
        case MUIF_MSG_CURSOR_ENTER:
            break;
        case MUIF_MSG_CURSOR_SELECT:
            // SELECT обрабатывается в display_task (сохранение яркости)
            return 1;
        case MUIF_MSG_CURSOR_UP:
            if (temp_brightness < 20) {
                temp_brightness++;
                set_contrast(temp_brightness * 12.75); // Масштабируем 1-20 в 0-255
                return 1;
            }
            break;
        case MUIF_MSG_CURSOR_DOWN:
            if (temp_brightness > 1) {
                temp_brightness--;
                set_contrast(temp_brightness * 12.75); // Масштабируем 1-20 в 0-255
                return 1;
            }
            break;
    }
    return 0;
}

// Список полей интерфейса (Field List) - это ядро вашего меню
muif_t menu_list[] = {
    MUIF_U8G2_FONT_STYLE(0, u8g2_font_helvR08_tr), /* normal text style */
    MUIF_U8G2_FONT_STYLE(1, u8g2_font_helvB08_tr), /* bold text style */
    MUIF_U8G2_FONT_STYLE(2, u8g2_font_profont12_tr), /* monospaced font */
    MUIF_U8G2_FONT_STYLE(3, u8g2_font_streamline_food_drink_t), /* food and drink */
    MUIF_U8G2_LABEL(),                               // Для меток (MUI_LABEL)
    MUIF_RO("HR", mui_hrline),                        // Горизонтальная линия
    MUIF_RO("MP",mui_u8g2_goto_data),
    MUIF_BUTTON("MC", mui_u8g2_goto_form_w1_pi),
    MUIF_RO("SP", mui_u8g2_goto_data),
    MUIF_U8G2_U16_LIST("SC", &list_selection1, NULL, menu_get_str, menu_get_cnt, mui_u8g2_u16_list_goto_w1_pi),
    MUIF_U8G2_U8_MIN_MAX_STEP("B0", &brightness, 0, 20, 1, MUI_MMS_2X_BAR|MUI_MMS_SHOW_VALUE, mui_u8g2_u8_bar_wm_mud_pi),
    MUIF_BUTTON("G0", mui_u8g2_btn_goto_wm_fi)
};

// Строки определения форм (Form Definition Strings)
fds_t menu_data[] =

// main menu
MUI_FORM(0)                  // Форма с ID 0
MUI_STYLE(1)                 // Установка стиля шрифта 0
MUI_LABEL(5, 10, "Menu")      // Метка " Menu" на (5,8)
MUI_XY("HR", 0, 13)          // Горизонтальная линия на y=10
MUI_STYLE(0) 
MUI_DATA("MP", 
    MUI_10 "GPS|"
    MUI_20 "LORA|"
    MUI_30 "Settings"
    )
MUI_XYA("MC", 5, 25, 0) 
MUI_XYA("MC", 5, 37, 1) 
MUI_XYA("MC", 5, 49, 2)

// settings menu
MUI_FORM(30)
MUI_STYLE(1)
MUI_LABEL(5, 10, "Settings")
MUI_XY("HR", 0, 13)
MUI_STYLE(0)
MUI_XYA("SC", 5, 25, 0) 
MUI_XYA("SC", 5, 37, 1)

// brightness settings form
MUI_FORM(35)
MUI_STYLE(1)
MUI_LABEL(5, 10, "Brightness")
MUI_XY("HR", 0, 13)
MUI_STYLE(0)
MUI_XY("B0", 40, 40)
MUI_XYAT("G0", 60, 59, 30, " Ok ")
;

size_t menu_list_size = sizeof(menu_list) / sizeof(muif_t);