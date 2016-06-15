/**
  ******************************************************************************
  * File Name          : serial.h
  * Description        : serial port io
  ******************************************************************************
  *
  * Copyright (c) 2016 S54MTB
  * Licensed under Apache License 2.0 
  * http://www.apache.org/licenses/LICENSE-2.0.html
  *
  ******************************************************************************
  */
#ifndef __serial_h__
#define __serial_h__


void process_rx_char(char rx_char);
void uart_puts(char *str);

#endif


