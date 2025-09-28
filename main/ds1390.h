#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

#ifndef DS1390_h
#define DS1390_h

// DS1390 SPI clock speed
#define DS1390_SPI_CLOCK        4000000

// Trickle charger modes
#define DS1390_TCH_DISABLE      0x00  // Disabled
#define DS1390_TCH_250_NO_D     0xA5  // 250 Ohms without diode
#define DS1390_TCH_250_D        0xA9  // 250 Ohms with diode
#define DS1390_TCH_2K_NO_D      0xA6  // 2 kOhms without diode
#define DS1390_TCH_2K_D         0xAA  // 2 kOhms with diode
#define DS1390_TCH_4K_NO_D      0xA7  // 4 kOhms without diode
#define DS1390_TCH_4K_D         0xAB  // 4 kOhms with diode

// Date formates
#define DS1390_FORMAT_24H       0     // 24h format
#define DS1390_FORMAT_12H       1     // 12h format

#define DS1390_AM               0     // AM
#define DS1390_PM               1     // PM

// DS1390 Адреса регистров для чтения данных
#define DS1390_ADDR_READ_HSEC   0x00  // Десятые доли секунд и сотые доли секунд
#define DS1390_ADDR_READ_SEC    0x01  // Секунды
#define DS1390_ADDR_READ_MIN    0x02  // Минуты
#define DS1390_ADDR_READ_HRS    0x03  // Часы
#define DS1390_ADDR_READ_WDAY   0x04  // Дни недели
#define DS1390_ADDR_READ_DAY    0x05  // Дни месяца
#define DS1390_ADDR_READ_MON    0x06  // Месяцы
#define DS1390_ADDR_READ_YRS    0x07  // Год
#define DS1390_ADDR_READ_AHSEC  0x08  // Alarm Hundredths of Seconds
#define DS1390_ADDR_READ_ASEC   0x09  // Alarm Seconds
#define DS1390_ADDR_READ_AMIN   0x0A  // Alarm Minutes
#define DS1390_ADDR_READ_AHRS   0x0B  // Alarm Hours
#define DS1390_ADDR_READ_ADAT   0x0C  // Alarm Day/Date
#define DS1390_ADDR_READ_CFG    0x0D  // Control - Used as SRAM in the DS1390
#define DS1390_ADDR_READ_STS    0x0E  // Status
#define DS1390_ADDR_READ_TCH    0x0F  // Trickle charger

// DS1390 Адреса регистров для записи данных
#define DS1390_ADDR_WRITE_HSEC  0x80  // Hundredths of Seconds
#define DS1390_ADDR_WRITE_SEC   0x81  // Seconds
#define DS1390_ADDR_WRITE_MIN   0x82  // Minutes
#define DS1390_ADDR_WRITE_HRS   0x83  // Hours
#define DS1390_ADDR_WRITE_WDAY  0x84  // Day of the week (1 = Sunday)
#define DS1390_ADDR_WRITE_DAY   0x85  // Day
#define DS1390_ADDR_WRITE_MON   0x86  // Month/Century
#define DS1390_ADDR_WRITE_YRS   0x87  // Year
#define DS1390_ADDR_WRITE_AHSEC 0x88  // Alarm Hundredths of Seconds
#define DS1390_ADDR_WRITE_ASEC  0x89  // Alarm Seconds
#define DS1390_ADDR_WRITE_AMIN  0x8A  // Alarm Minutes
#define DS1390_ADDR_WRITE_AHRS  0x8B  // Alarm Hours
#define DS1390_ADDR_WRITE_ADAT  0x8C  // Alarm WDay/Day
#define DS1390_ADDR_WRITE_CFG   0x8D  // Control - Used as SRAM in the DS1390
#define DS1390_ADDR_WRITE_STS   0x8E  // Status
#define DS1390_ADDR_WRITE_TCH   0x8F  // Trickle charger

// DS1390 register bit masks
#define DS1390_MASK_AMPM        0x20  // AM/PM bit
#define DS1390_MASK_FORMAT      0x40  // 12h/24h format bit
#define DS1390_MASK_CENTURY     0x80  // Century bit
#define DS1390_MASK_OSF         0x80  // Oscillator stop flag bit
#define DS1390_MASK_AMX         0x80  // Alarm  bit (x = 1-4)
#define DS1390_MASK_DYDT        0x40  // Alarm day/date bit

// Leap year calulator
#define LEAP_YEAR(Y)            (((1970+(Y))>0) && !((1970+(Y))%4) && (((1970+(Y))%100) || !((1970+(Y))%400)))

/* ------------------------------------------------------------------------------------------- */
// Structures
/* ------------------------------------------------------------------------------------------- */

// DS1390 date and time store fields
typedef struct {
  uint8_t hundredths; // Hundredths of Seconds
  uint8_t seconds;    // Seconds
  uint8_t minutes;    // Minutes
  uint8_t hours;      // Hours
  } ds1390_time_t;

void ds1390_read_time(ds1390_time_t *time);
void ds1390_read_status(uint8_t *status);
void ds1390_write_time(const ds1390_time_t *time);
uint8_t ds1390_read_from_reg(uint8_t reg_addr);
void ds1390_write_to_reg(uint8_t data, uint16_t register);
void ds1390_enable_trickle_charger(void);
void ds1390_enable_oscillator(void);

#endif