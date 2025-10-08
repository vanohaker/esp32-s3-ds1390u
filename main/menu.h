#ifndef MENU_H
#define MENU_H

#include <u8g2.h>
#include <mui.h>
#include <mui_u8g2.h>

// Глобальные переменные
extern muif_t menu_list[];
extern size_t menu_list_size;
extern fds_t menu_data[];

// Функции
uint8_t mui_hrule(mui_t *mui, uint8_t msg);
uint8_t mui_button(mui_t *mui, uint8_t msg);
uint8_t mui_nav_button(mui_t *mui, uint8_t msg);

#endif // MENU_H