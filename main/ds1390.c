#include "ds1390.h"
#include "spi_config.h"
#include "driver/spi_master.h"
#include <stdio.h>

void ds1390_read_time(ds1390_time_t *time) {
    time->hundredths = ds1390_read_from_reg(DS1390_ADDR_READ_HSEC);
    time->seconds = ds1390_read_from_reg(DS1390_ADDR_READ_SEC);
    time->minutes = ds1390_read_from_reg(DS1390_ADDR_READ_MIN);
    time->hours = ds1390_read_from_reg(DS1390_ADDR_READ_HRS);
}

void ds1390_read_status(uint8_t *status) {
    *status = ds1390_read_from_reg(DS1390_ADDR_READ_STS);
}

void ds1390_write_time(const ds1390_time_t *time) {
    ds1390_write_to_reg(time->hundredths, DS1390_ADDR_WRITE_HSEC);
    ds1390_write_to_reg(((time->seconds / 10) << 4) | (time->seconds % 10), DS1390_ADDR_WRITE_SEC);
    ds1390_write_to_reg(((time->minutes / 10) << 4) | (time->minutes % 10), DS1390_ADDR_WRITE_MIN);
    ds1390_write_to_reg(((time->hours / 10) << 4) | (time->hours % 10), DS1390_ADDR_WRITE_HRS);
}

void ds1390_enable_trickle_charger(void) {
    ds1390_write_to_reg(0xA5, DS1390_ADDR_WRITE_TCH);
}

void ds1390_enable_oscillator(void) {
    ds1390_write_to_reg(0x00, DS1390_ADDR_WRITE_CFG);
}

uint8_t ds1390_read_from_reg(uint8_t reg_addr) {
    uint8_t data;
    spi_transaction_t t = {
        .tx_buffer = NULL,
        .rx_buffer = &data,
        .length = 8,
        .cmd = reg_addr,
    };
    ESP_ERROR_CHECK(spi_device_polling_transmit(spi_get_handle(), &t));
    printf("read addr: %02X, data: %02X\n", reg_addr, data);
    return data;
}

void ds1390_write_to_reg(uint8_t data, uint16_t reg_addr) {
    spi_transaction_t t = {
        .tx_buffer = &data,
        .rx_buffer = NULL,
        .length = 8,
        .cmd = reg_addr,
    };
    printf("write addr: %02X, data: %02X\n", reg_addr, data);
    ESP_ERROR_CHECK(spi_device_polling_transmit(spi_get_handle(), &t));
}