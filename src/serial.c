/**
  ******************************************************************************
  * File Name          : serial.c
  * Description        : Serial buffer handling
  ******************************************************************************
  *
  * Copyright (c) 2016 S54MTB
  * Licensed under Apache License 2.0 
  * http://www.apache.org/licenses/LICENSE-2.0.html
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
#include <string.h>
#include "serial.h"
#include "setup.h"

extern UART_HandleTypeDef huart2;
extern void hdlc_process_rx_byte(uint8_t rx_byte);

/**
 * Process received char, check if LF or CR received
 * Set flag when line is done
 */
void process_rx_char(char rx_char)
{
  hdlc_process_rx_byte(rx_char);
}


void uart_puts(char *str)
{
	uint16_t len = strlen(str);
	HAL_UART_Transmit_IT(&huart2, (uint8_t *)str, len);
	HAL_Delay(len*2);  // foolproof
}


void uart_putchar(char ch)
{
	HAL_UART_Transmit_IT(&huart2, (uint8_t *)&ch, 1);
	HAL_Delay(2);
}


