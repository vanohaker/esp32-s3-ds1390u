// gps.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "driver/uart.h"
#include "gps.h"
#include "menu.h"

#define GPS_UART_NUM UART_NUM_2
#define GPS_TXD 17
#define GPS_RXD 16
#define GPS_BAUDRATE 9600
#define GPS_BUF_SIZE 1024

void init_gps(void) {
    uart_config_t uart_config = {
        .baud_rate = GPS_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(GPS_UART_NUM, &uart_config);
    uart_set_pin(GPS_UART_NUM, GPS_TXD, GPS_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(GPS_UART_NUM, GPS_BUF_SIZE, 0, 0, NULL, 0);
}

void parse_nmea_gps(char *nmea_str) {
    if (strncmp(nmea_str, "$GPGGA,", 7) == 0) {
        char *token = strtok(nmea_str + 7, ",");
        int field = 0;
        char lat_dir, lon_dir;
        double lat_deg, lon_deg, alt;

        while (token != NULL) {
            field++;
            switch (field) {
                case 2: // Latitude
                    lat_deg = atof(token) / 100.0;
                    lat_dir = token[strlen(token) - 1];
                    break;
                case 4: // Longitude
                    lon_deg = atof(token) / 100.0;
                    lon_dir = token[strlen(token) - 1];
                    break;
                case 7: // Satellites
                    gps_data.satellites = atoi(token);
                    break;
                case 9: // Altitude
                    alt = atof(token);
                    break;
            }
            token = strtok(NULL, ",");
        }

        if (field >= 6) {
            gps_data.latitude = lat_deg + (lat_deg - (int)lat_deg) * 100.0 / 60.0;
            if (lat_dir == 'S') gps_data.latitude = -gps_data.latitude;
            gps_data.longitude = lon_deg + (lon_deg - (int)lon_deg) * 100.0 / 60.0;
            if (lon_dir == 'W') gps_data.longitude = -gps_data.longitude;
            gps_data.altitude = alt;
            gps_data.valid = true;
        }
    } else if (strncmp(nmea_str, "$GPRMC,", 7) == 0) {
        char *token = strtok(nmea_str + 7, ",");
        int field = 0;
        struct tm gps_tm = {0};

        while (token != NULL) {
            field++;
            switch (field) {
                case 1: // Time (HHMMSS.sss)
                    gps_tm.tm_hour = atoi(token) / 10000;
                    gps_tm.tm_min = (atoi(token) % 10000) / 100;
                    gps_tm.tm_sec = atoi(token) % 100;
                    break;
                case 9: // Date (DDMMYY)
                    gps_tm.tm_mday = atoi(token) / 10000;
                    gps_tm.tm_mon = (atoi(token) % 10000) / 100 - 1;
                    gps_tm.tm_year = atoi(token) % 100 + 2000 - 1900;
                    break;
                case 2: // Status (A=valid, V=invalid)
                    if (token[0] == 'A') gps_data.time_valid = true;
                    else gps_data.time_valid = false;
                    break;
            }
            token = strtok(NULL, ",");
        }
        if (gps_data.time_valid) {
            gps_data.gps_time = gps_tm;
        }
    }
}

void gps_task(void *pvParameters) {
    uint8_t data[GPS_BUF_SIZE];
    char nmea_buf[256];
    int nmea_len = 0;

    while (1) {
        int len = uart_read_bytes(GPS_UART_NUM, data, sizeof(data), 100 / portTICK_PERIOD_MS);
        if (len > 0) {
            for (int i = 0; i < len; i++) {
                if (data[i] == '\n') {
                    nmea_buf[nmea_len] = '\0';
                    parse_nmea_gps(nmea_buf);
                    nmea_len = 0;
                } else if (nmea_len < sizeof(nmea_buf) - 1) {
                    nmea_buf[nmea_len++] = data[i];
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}