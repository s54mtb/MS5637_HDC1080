/* 
  ******************************************************************************
  * File Name          : MS5637.c
  * Description        : MS5637 driver
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 s54mtb
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  *
  ******************************************************************************

	 The MS5637 communicates with the host controller over a digital I2C 
   interface. The 7-bit base slave address is 0x76
	 
*/

#include "stm32f0xx_hal.h" 
#include "MS5637.h"
#include <string.h>
#include <math.h>

#define MS5637_ADDR 0x76
#define MS5637_SKIP_CRC 1


/*
 * MS5637_reset() - Read User register
 * @hi2c:  handle to I2C interface
 * Returns HAL status or HAL_ERROR for invalid parameters.
 */
HAL_StatusTypeDef MS5637_reset(I2C_HandleTypeDef *hi2c)
{
	uint8_t cmd = MS5637_CMD_RESET;

	/* Send the reset command */
	return HAL_I2C_Master_Transmit(hi2c,MS5637_ADDR<<1,&cmd,1,100);	
}

/*
 * MS5637_read_PROM() - Read valuse from PROM
 * @hi2c:  handle to I2C interface
 * @addr: PROM address
 * @val: 16-bit PROM value from the MS5637
 * Returns HAL status or HAL_ERROR for invalid parameters.
 */
HAL_StatusTypeDef MS5637_read_PROM(I2C_HandleTypeDef *hi2c, uint8_t addr, uint16_t *val)
{
	uint8_t buf[2];
	HAL_StatusTypeDef  error;

	// Check argument
	if (addr>7)
		return HAL_ERROR;
	
	buf[0] = MS5637_PROM_READ | (addr << 1);
	/* Read register */
	/* Send the read followed by address */
	error = HAL_I2C_Master_Transmit(hi2c,MS5637_ADDR<<1,buf,1,100);
	if (error != HAL_OK)
		return error;

	HAL_Delay(1); 
	
	/* Receive a 2-byte result */
	error = HAL_I2C_Master_Receive(hi2c, MS5637_ADDR<<1 | 0x01, buf, 2, 1000);
	if (error != HAL_OK)
		return error;
	
	/* Result */
	*val = buf[0]*256+buf[1]; 

	return HAL_OK;  /* Success */
	
}


/*
 * MS5637_read_ADC_TP() - Read ADC
 * @hi2c		:  handle to I2C interface
 * @channel	: MS5637_CONVERT_D1_BASE or MS5637_CONVERT_D2_BASE
 * @osr			: oversampling ratio for MS5637 ADC conversion
 * @val 		: ADC result (24 bit)
 * Returns HAL status or HAL_ERROR for invalid parameters.
 */
HAL_StatusTypeDef MS5637_read_ADC_TP(I2C_HandleTypeDef *hi2c, uint8_t channel, uint8_t osr, uint32_t *val)
{
	uint8_t buf[3];
	HAL_StatusTypeDef  error;

	// Check argument
	if ((osr != MS5637_OSR_256) &
		  (osr != MS5637_OSR_512) &
		  (osr != MS5637_OSR_1024) &
		  (osr != MS5637_OSR_2048) &
		  (osr != MS5637_OSR_4096) &
	    (osr != MS5637_OSR_8192) )
		return HAL_ERROR;
	
	if ((channel != MS5637_CONVERT_D1_BASE) &
	    (channel != MS5637_CONVERT_D2_BASE) )
		return HAL_ERROR;
	
	buf[0] = channel | osr;
	/* Send the read followed by cmd */
	error = HAL_I2C_Master_Transmit(hi2c,MS5637_ADDR<<1,buf,1,100);
	if (error != HAL_OK)
		return error;

	HAL_Delay(10*osr); 
	
	buf[0] = MS5637_ADC_READ;
	/* Send the read followed by cmd */
	error = HAL_I2C_Master_Transmit(hi2c,MS5637_ADDR<<1,buf,1,100);
	if (error != HAL_OK)
		return error;
	
	HAL_Delay(osr); 
	
	/* Receive a 3-byte result */
	error = HAL_I2C_Master_Receive(hi2c, MS5637_ADDR<<1 | 0x01, buf, 3, 1000);
	if (error != HAL_OK)
		return error;
	
	/* Result */
	*val = buf[0]*256*256+buf[1]*256+buf[2]; 

	return HAL_OK;  /* Success */
	
}



/*
 * MS5637_Calculate() - Calculate pressure and temperature form calibration coefficients 
 * @C				: Calibration coefficients, equal memory map as in MS5637
 * @D1			: ADC Readout from MS5637_CONVERT_D1_BASE 
 * @D2			: ADC Readout from MS5637_CONVERT_D2_BASE
 * @Temperature 		: Calculated MS5637 sensor temperature in °C
 * @Pressure    		: Calculated MS5637 sensor pressure in mBar
 * Returns HAL status or HAL_ERROR for invalid parameters.
 */
HAL_StatusTypeDef MS5637_Calculate(uint16_t *C, uint32_t D1, uint32_t D2, double *Temperature, double *Pressure)
{
	unsigned char nCRC4;       // check sum to ensure PROM integrity
	double dT, OFFSET, SENS, T2, OFFSET2, SENS2;  // Raw data for calculation
	
	unsigned char refCRC4 = (C[0] >> 12) & 0x000f;

	nCRC4 = MS5637_checkCRC4(C);
	if (nCRC4 != refCRC4)  // CRC check not passed
		#ifdef MS5637_SKIP_CRC
		return HAL_ERROR
		#endif
		;
	
	dT = D2 - C[5]*pow(2,8);    // Calculate dT, OFS and SENS, datasheet page 6
  OFFSET = C[2]*pow(2, 17) + dT*C[4]/pow(2,6);
  SENS = C[1]*pow(2,16) + dT*C[3]/pow(2,7);
 
  *Temperature = (2000 + ((dT*C[6]))/pow(2, 23))/100;  // MS5637 temp has 0.01°C "unit" - convert to 1°C 
//-------------------------------------------
// Calculate second order corrections
	if(*Temperature > 20.0f) 
	{
		T2 = 5*dT*dT/pow(2, 38); // correction above 20°C
		OFFSET2 = 0;
		SENS2 = 0;
	}
	if(*Temperature < 20.0f)   // correction below 20°C
	{
		T2      = 3*dT*dT/pow(2, 33); 
		OFFSET2 = 61*(100 * *Temperature - 2000)*(100 * *Temperature - 2000)/16;
		SENS2   = 29*(100 * *Temperature - 2000)*(100 * *Temperature - 2000)/16;
	} 
	if(*Temperature < -15.0f)     // correction below -15°C
	{
		OFFSET2 = OFFSET2 + 17*(100 * *Temperature + 1500)*(100 * *Temperature + 1500);
		SENS2 = SENS2 + 9*(100 * *Temperature + 1500)*(100 * *Temperature + 1500);
  }
 // End of second order corrections
 //-------------------------------------------
	*Temperature = *Temperature - T2/100;
	OFFSET = OFFSET - OFFSET2;
	SENS = SENS - SENS2;
	
	// Pressure in mbar or hPa
	*Pressure = (((D1*SENS)/pow(2, 21) - OFFSET)/pow(2, 15))/100; 

	if ( (*Temperature < -40.0f) | (*Temperature > 85.0f) |
		   (*Pressure < 10.0f) | (*Pressure > 2000.0f))
	return 
		HAL_ERROR;
	else	
	  return HAL_OK;
	
}




/*
 * MS5637_checkCRC4() - calculate checksum form calibration coefficients 
 * @C				: Calibration coefficients, equal memory map as in MS5637
 * Returns CRC-4 calculation from C[]
 */
unsigned char MS5637_checkCRC4(uint16_t * C)
{
  int cnt;
  unsigned int n_rem = 0;
  unsigned char n_bit;
  
  C[0] = ((C[0]) & 0x0FFF);  // replace CRC byte by 0 for checksum calculation
  C[7] = 0;
  for(cnt = 0; cnt < 16; cnt++)
  {
    if(cnt%2==1) n_rem ^= (unsigned short) ((C[cnt>>1]) & 0x00FF);
    else         n_rem ^= (unsigned short)  (C[cnt>>1]>>8);
    for(n_bit = 8; n_bit > 0; n_bit--)
    {
        if(n_rem & 0x8000)    n_rem = (n_rem<<1) ^ 0x3000;
        else                  n_rem = (n_rem<<1);
    }
  }
  n_rem = ((n_rem>>12) & 0x000F);
  return (n_rem ^ 0x00);
}

