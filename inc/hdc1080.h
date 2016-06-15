
#ifndef __hdc1080_h__
#define __hdc1080_h__

#include "stm32f0xx_hal.h"

/* Register addresses */
#define HDC1080_TEMPERATURE				0x00
#define HDC1080_HUMIDITY 					0x01
#define HDC1080_CONFIG						0x02
#define HDC1080_SERIAL_ID1				0xfb
#define HDC1080_SERIAL_ID2				0xfc
#define HDC1080_SERIAL_ID3				0xfd
#define HDC1080_ID_MANU						0xfe
#define HDC1080_ID_DEV						0xff

#define HDC1080_RH_RES_14					0x00
#define HDC1080_RH_RES_11					0x01
#define HDC1080_RH_RES8						0x02

#define HDC1080_T_RES_14					0x00
#define HDC1080_T_RES_11					0x01 


HAL_StatusTypeDef hdc1080_read_reg(I2C_HandleTypeDef *hi2c, uint16_t delay, uint8_t reg, uint16_t *val);
HAL_StatusTypeDef hdc1080_write_reg(I2C_HandleTypeDef *hi2c, uint8_t reg, uint16_t val);
HAL_StatusTypeDef hdc1080_measure(I2C_HandleTypeDef *hi2c,   uint8_t temp_res, uint8_t humidres, uint8_t heater, 	uint8_t *bat_stat, double *temperature,	double *humidity);
HAL_StatusTypeDef hdc1080_get_device_id(I2C_HandleTypeDef *hi2c, uint64_t *serial, uint16_t *manuf, uint16_t *device);

#endif
