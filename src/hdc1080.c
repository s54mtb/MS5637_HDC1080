/* 
  ******************************************************************************
  * File Name          : hdc1080.c
  * Description        : hdc1080 driver
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 s54mtb
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  *
  ******************************************************************************

	 The hdc1080 communicates with the host controller over a digital I2C 
   interface. The 7-bit base slave address is 0x40 or 0x41
	 
*/

#include "stm32f0xx_hal.h" 
#include "hdc1080.h"
#include <string.h>

#define HDC1080_ADDR 0x40 


/*
 * hdc1080_read_reg() - Read User register
 * @hi2c:  handle to I2C interface
 * @delay: Delay before read
 * @reg: Register address
 * @val: 16-bit register value from the hdc1080
 * Returns HAL status or HAL_ERROR for invalid parameters.
 */
HAL_StatusTypeDef hdc1080_read_reg(I2C_HandleTypeDef *hi2c, uint16_t delay, uint8_t reg, uint16_t *val)
{
	uint8_t buf[2];
	HAL_StatusTypeDef  error;

	// Check argument
	if ((reg != HDC1080_TEMPERATURE) &
		  (reg != HDC1080_HUMIDITY) &
		  (reg != HDC1080_CONFIG) &
		  (reg != HDC1080_SERIAL_ID1) &
		  (reg != HDC1080_SERIAL_ID2) &
		  (reg != HDC1080_SERIAL_ID3) &
		  (reg != HDC1080_ID_MANU) &
	    (reg != HDC1080_ID_DEV) )
		return HAL_ERROR;
	
	buf[0] = reg;
	/* Read register */
	/* Send the read followed by address */
	error = HAL_I2C_Master_Transmit(hi2c,HDC1080_ADDR<<1,buf,1,100);
	if (error != HAL_OK)
		return error;

	HAL_Delay(delay); 
	
	/* Receive a 2-byte result */
	error = HAL_I2C_Master_Receive(hi2c, HDC1080_ADDR<<1 | 0x01, buf, 2, 1000);
	if (error != HAL_OK)
		return error;
	
	/* Result */
	*val = buf[0]*256+buf[1]; 

	return HAL_OK;  /* Success */
	
}


/*
 * hdc1080_write_reg() - Write register
 * @hi2c:  handle to I2C interface
 * @reg: Register address
 * @val: 8-bit register value from the Si7013
 * Returns HAL status or HAL_ERROR for invalid arguments.
 */
HAL_StatusTypeDef hdc1080_write_reg(I2C_HandleTypeDef *hi2c, uint8_t reg, uint16_t val)
{
	uint8_t buf[3];
	HAL_StatusTypeDef  error;

		// Check argument
	if ((reg != HDC1080_TEMPERATURE) &  // dummy write to adr 0 ... trigger measurement
		  (reg != HDC1080_CONFIG) )       // config is "writable"
		return HAL_ERROR;

	buf[0] = reg;
	buf[1] = (uint8_t)((val >> 8) & 0xff);  // msb
	buf[2] = (uint8_t)(val & 0xff); 				// lsb
	/* Write the register */
	/* Send the command and data */
	error = HAL_I2C_Master_Transmit(hi2c,HDC1080_ADDR<<1,buf,3,100);
	if (error != HAL_OK)
		return error;
  else 
	  return HAL_OK;  /* Success */
}




/*
 * hdc1080_measure() - measure humididty and temperature: 

1. Configure the acquisition parameters in config register (address 0x02):
		(a) Set the acquisition mode to measure both temperature
				and humidity by setting Bit[12] to 1.
		(b) Set the desired temperature measurement resolution:
				–  Set Bit[10] to 0 for 14 bit resolution.
				–  Set Bit[10] to 1 for 11 bit resolution.
		(c) Set the desired humidity measurement resolution:
				–  Set Bit[9:8] to 00 for 14 bit resolution.
				–  Set Bit[9:8] to 01 for 11 bit resolution.
				–  Set Bit[9:8] to 10 for 8 bit resolution.

2. Trigger the measurements by writing I2C read reg. address with adr = HDC1080_TEMPERATURE.

3. Wait for the measurements to complete, based on the conversion time

4. Read the output data:
Read the temperature data from register address 0x00, followed by the humidity 
data from register address 0x01 in a single transaction. A read operation will 
return a NACK if the contents of the registers have not been updated.

 * @hi2c:  handle to I2C interface
 * @temp_res    :  temperature measurement resolution:
 *										- HDC1080_T_RES_14 or
 *                    - HDC1080_T_RES_11
 * @humidres    :  humidity readout resolution
 *                    - HDC1080_RH_RES_14 or
 *										- HDC1080_RH_RES_11 or
 *										- HDC1080_RH_RES_8 
 * @heater      :  heater enable (0 = disabled or 1 = enabled)
 * @bat_stat    :  supply voltage 
 *                    - 0 when Ucc > 2,8V
 *										- 1 when Ucc < 2,8V
 * @temperature :  floating point temperature result, unit is °C
 * @humidity    :  floating point humidity result, unit is RH%
 * Returns HAL status.
 */
HAL_StatusTypeDef hdc1080_measure(I2C_HandleTypeDef *hi2c,
  uint8_t temp_res, uint8_t humidres, uint8_t heater,
	uint8_t *bat_stat, double *temperature,	double *humidity)
{
	HAL_StatusTypeDef error;
	uint16_t r;
	double tmp;
	
	
	error = hdc1080_read_reg(hi2c, 10, HDC1080_CONFIG, &r);
	if (error != HAL_OK) return error;

	r |= temp_res<<10;
	r |= humidres<<8;
	r |= 1<<12;     // mode = 1;
	r |= heater<<13;
	
	*bat_stat = (r>>11) & 0x0001;
	
	// write config
	error = hdc1080_write_reg(hi2c, HDC1080_CONFIG, r);
	if (error != HAL_OK) return error;
	

	error = hdc1080_read_reg(hi2c, 150, HDC1080_TEMPERATURE, &r);
	if (error != HAL_OK) return error;
	tmp = (double)r;
	tmp = (tmp / 65536.0f) * 165.0f - 40.0f;
	*temperature = tmp;  // °C
	
	error = hdc1080_read_reg(hi2c, 150, HDC1080_HUMIDITY, &r);
	tmp = (int32_t)r;
	
	if (error != HAL_OK) return error;
	tmp = (float)r;
	tmp = (tmp / 65536.0f) * 100.0f;
	if (tmp>100.0) tmp = 100.0f;
	if (tmp<0) tmp = 0.0f;
	*humidity = tmp;
	
	return HAL_OK;
}


/*
 * hdc1080_get_device_id() - Get the device ID from the device
 * @i2c: handle to I2C interface
 * @serial: 40 bit serial number
 * @manuf: 16 bit manufacturer ID
 * @device: 16 bit device ID
 *
 * Returns 0 on success.
 */
HAL_StatusTypeDef hdc1080_get_device_id(I2C_HandleTypeDef *hi2c, uint64_t *serial, uint16_t *manuf, uint16_t *device)
{
  uint16_t ser[4];
	HAL_StatusTypeDef error;

	error = hdc1080_read_reg(hi2c, 10, HDC1080_SERIAL_ID1, &ser[0]);
	if (error != HAL_OK) return error;
	
	error = hdc1080_read_reg(hi2c, 10, HDC1080_SERIAL_ID2, &ser[1]);
	if (error != HAL_OK) return error;
	
	error = hdc1080_read_reg(hi2c, 10, HDC1080_SERIAL_ID3, &ser[2]);
	if (error != HAL_OK) return error;
	
	ser[3] = 0;
	memcpy(serial, ser, 8);
	
	error = hdc1080_read_reg(hi2c, 10, HDC1080_ID_MANU, manuf);
	if (error != HAL_OK) return error;

	error = hdc1080_read_reg(hi2c, 10, HDC1080_ID_DEV, device);
	if (error != HAL_OK) return error;
	
	return HAL_OK;  /* Success */
}



