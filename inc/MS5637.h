
#ifndef __MS5637_h__
#define __MS5637_h__

#include "stm32f0xx_hal.h"

/* Register addresses */
#define MS5637_CMD_RESET				0x1E
#define MS5637_CONVERT_D1_BASE  0x40
#define MS5637_CONVERT_D2_BASE  0x50
#define MS5637_OSR_256				  0x00
#define MS5637_OSR_512				  0x02
#define MS5637_OSR_1024				  0x04
#define MS5637_OSR_2048				  0x06
#define MS5637_OSR_4096				  0x08
#define MS5637_OSR_8192				  0x0A
#define MS5637_ADC_READ				  0x00
#define MS5637_PROM_READ			  0xA0


HAL_StatusTypeDef MS5637_reset(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef MS5637_read_PROM(I2C_HandleTypeDef *hi2c, uint8_t addr, uint16_t *val);
HAL_StatusTypeDef MS5637_read_ADC_TP(I2C_HandleTypeDef *hi2c, uint8_t channel, uint8_t osr, uint32_t *val);
HAL_StatusTypeDef MS5637_Calculate(uint16_t *C, uint32_t D1, uint32_t D2, double *Temperature, double *Pressure);
unsigned char MS5637_checkCRC4(uint16_t * C);

#endif
