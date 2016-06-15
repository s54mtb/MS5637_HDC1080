#include "stm32f0xx.h"                  // Device header
#include "uuid.h"
#include <string.h>

#include "payload_processor.h"
#include "setup.h"
#include "hdlc.h"
#include "MS5637.h"
#include "hdc1080.h"


enum
{
	CMD_Temperature = 0x40, /// Temperature readout from hdc1080
	CMD_Humidity,     			/// Humidity readout from hdc1080
	CMD_Bat,								/// battery readout from hdc1080
	CMD_Pressure,						/// Air pressure in hPa or mbar abs
	CMD_pTemperature,       /// Temperature readout from pressure sensor
	CMD_pCAL,       				/// Calibration coefficients from pressure sensor
	CMD_pD1,       					/// Raw pressure readout from pressure sensor
	CMD_pD2,       					/// Raw temperature from pressure sensor
	CMD_ID,									/// Identification
};

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart2;

int16_t payload_processor(hdlc_t *hdlc)
{	
	uint8_t response[24],i;
  int16_t len=0;
	uint32_t uid = UNIQUE_ID;
	uint16_t Pcal[8];         // calibration constants from MS5637 PROM registers
	double hum, temp;
	uint8_t bat; 
	uint32_t D1 = 0, D2 = 0;  // raw MS5637 pressure and temperature data
	double Temperature, Pressure; // stores MS5637 pressures sensor pressure and temperature	
	
	
	// Commands for HDC1080
	if ((hdlc->p_payload[0] == CMD_Temperature) |
		  (hdlc->p_payload[0] == CMD_Humidity) |
		  (hdlc->p_payload[0] == CMD_Bat))
	{
		hdc1080_measure(&hi2c1, HDC1080_T_RES_14, HDC1080_RH_RES_14, 0, &bat, &temp, &hum);
	}
	
	// Commands for MS5637
	if ((hdlc->p_payload[0] == CMD_Pressure) |
	    (hdlc->p_payload[0] == CMD_pTemperature) |
	    (hdlc->p_payload[0] == CMD_pCAL) |
	    (hdlc->p_payload[0] == CMD_pD1) |
	    (hdlc->p_payload[0] == CMD_pD2) )
	{
		for (i=0; i<8; i++)
		{
			MS5637_read_PROM(&hi2c1, i, &Pcal[i]); // c1 to c8 --- 
		}
		
		MS5637_read_ADC_TP(&hi2c1, MS5637_CONVERT_D1_BASE, MS5637_OSR_8192, &D1);  // D1
		MS5637_read_ADC_TP(&hi2c1, MS5637_CONVERT_D2_BASE, MS5637_OSR_8192, &D2);  // D2
		MS5637_Calculate(Pcal, D1, D2, &Temperature, &Pressure);
	}
	 
	switch (hdlc->p_payload[0])
	{
		case CMD_Temperature :			
		  memcpy(response, &temp, sizeof(double));
			len=sizeof(double)+1;
		break;
		
		case CMD_Humidity :
		  memcpy(response, &hum, sizeof(double));
			len=sizeof(double)+1;
		break;
		
    case CMD_Bat:
		  response[0] = bat;
			len=2;			
		break;
		
    case CMD_Pressure:
		  memcpy(response, &Pressure, sizeof(double));
			len=sizeof(double)+1;
		break;
		
		case CMD_pTemperature:
		  memcpy(response, &Temperature, sizeof(double));
			len=sizeof(double)+1;
		break;
		
		case CMD_pCAL:
		  memcpy(response, Pcal, 2*8);
			len=2*8+1;
		break;
		
		case CMD_pD1:
		  memcpy(response, &D1, 2);
			len=2+1;
		break;
		
		case CMD_pD2:
		  memcpy(response, &D2, 2);
			len=2+1;
		break;
		
		case CMD_ID:
		  memcpy(&response[1],&uid,4);
			len=6;						
		break;	
		
	}
	
	for (i=0; i<len; i++) hdlc->p_payload[i+1]=response[i];

	return len;
}




