/**
  ******************************************************************************
  * File Name          : hdlc.c
  * Description        : HDLC frame parser
  ******************************************************************************
  *
  * Copyright (c) 2016 S54MTB
  * Licensed under Apache License 2.0 
  * http://www.apache.org/licenses/LICENSE-2.0.html
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"          // Device header, mostly for uintX_t defines
#include "hdlc.h"								// header for this HDLC implementation
#include <string.h>							// memcpy()
#include "crc.h"


// Basic setup constants, define for debugging and skip CRC checking, too...
#include "setup.h"


__weak void uart_putchar(char ch)
{
	// implement function to send character via UART
}

//extern void uart_putchar(char ch);
__weak int16_t payload_processor(hdlc_t *hdlc)
{
	// implement function to do something with the received payload
	// when this function return > 0 ... the HDLC frame processor will 
	// initiate sending of the payload back to the device with source address
	return 0;
}

static hdlc_t		hdlc;

// Static buffer allocations
static uint8_t  _hdlc_rx_frame[HDLC_MRU];   // rx frame buffer allocation
static uint8_t  _hdlc_tx_frame[HDLC_MRU];   // tx frame buffer allocation
static uint8_t  _hdlc_payload[HDLC_MRU];    // payload buffer allocation


/** Private functions to send bytes via UART */
/* Send a byte via uart_putchar() function */
static void hdlc_tx_byte(uint8_t byte)
{
  uart_putchar((char)byte);
}

/* Check and send a byte with hdlc ESC sequence via UART */
static void hdlc_esc_tx_byte(uint8_t byte)
{
	if((byte == HDLC_CONTROL_ESCAPE) || (byte == HDLC_FLAG_SOF))
	{
		hdlc_tx_byte(HDLC_CONTROL_ESCAPE);
		byte ^= HDLC_ESCAPE_BIT;
		hdlc_tx_byte(byte);
	}
	else 
		hdlc_tx_byte(byte);
}


/* initialiyatiuon of the HDLC state machine, buffer pointers and status variables */
void hdlc_init(void)
{
  hdlc.rx_frame_index = 0;
  hdlc.rx_frame_fcs   = HDLC_CRC_INIT_VAL;
	hdlc.p_rx_frame 	  = _hdlc_rx_frame;
	memset(hdlc.p_rx_frame, 0, HDLC_MRU);
	hdlc.p_tx_frame 	  = _hdlc_tx_frame;
	memset(hdlc.p_tx_frame, 0, HDLC_MRU);
	hdlc.p_payload 	    = _hdlc_payload;
	memset(hdlc.p_payload, 0, HDLC_MRU);
	hdlc.state					= HDLC_SOF_WAIT;
	hdlc.own_addr				= SETUP_OWNADDRESS;
}


/* This function should be called when new character is received via UART */
void hdlc_process_rx_byte(uint8_t rx_byte)
{
	switch (hdlc.state)
	{
		case HDLC_SOF_WAIT:   /// Waiting for SOF flag
			if (rx_byte == HDLC_FLAG_SOF) 
			{
				hdlc_init();
				hdlc.state = HDLC_DATARX;
			}
		break;
			
		case HDLC_DATARX:     /// Data reception process running
			if (rx_byte == HDLC_CONTROL_ESCAPE)  // is esc received ?
			{
				hdlc.state = HDLC_PROC_ESC;  // handle ESCaped byte
				break;
			} 
			// not ESC, check for next sof
			if (rx_byte == HDLC_FLAG_SOF) // sof received ... process frame
			{
				if (hdlc.rx_frame_index == 0) // sof after sof ... drop and continue
					break;
				if (hdlc.rx_frame_index > 5) // at least addresses + crc
				  hdlc_process_rx_frame(hdlc.p_rx_frame, hdlc.rx_frame_index);
				hdlc_init();
				hdlc.state = HDLC_DATARX;
			} else // "normal" - not ESCaped byte
			{
				if (hdlc.rx_frame_index < HDLC_MRU) 
				{
					hdlc.p_rx_frame[hdlc.rx_frame_index] = rx_byte;
					hdlc.rx_frame_index++;					
				} else // frame overrun
				{
					hdlc_init();   // drop frame and start over
				}
			}
		break;
			
		case HDLC_PROC_ESC:  /// process ESCaped byte
			hdlc.state = HDLC_DATARX; // return to normal reception after this
		  if (hdlc.rx_frame_index < HDLC_MRU) // check for overrun
			{
				rx_byte ^= HDLC_ESCAPE_BIT;  // XOR with ESC bit
				hdlc.p_rx_frame[hdlc.rx_frame_index] = rx_byte;  
				hdlc.rx_frame_index++;
			} else // frame overrun
			{
				hdlc_init();  // drop frame and start over
			}
		break;
			
			
	}
}

/** Process received frame buf with length len
  Frame structure:
	  [Source Address]				// Address of the data source
		[Destination address]		// Address of the data destination
		[HDLC Ctrl byte]				// only UI - Unnumbered Information with payload are processed
		[payload]								// 1 or more bytes of payload data
		[crc16-H]								// MSB of CRC16
		[crc16-L]								// LSB of CRC16
*/
void hdlc_process_rx_frame(uint8_t *buf, uint16_t len)
{
	if (len>=5)               // 5 bytes overhead (2xaddr+ctrl+crc)
	{
		hdlc.src_addr = buf[0];		// source address --- nedded for sending reply
		hdlc.dest_addr = buf[1];  // destination address --- check for match with own address
		hdlc.ctrl = buf[2];				// HDLC Ctrl byte
		
		// Is the received packet for this device and has proper ctrl ?
		if ((hdlc.dest_addr == SETUP_OWNADDRESS) & (hdlc.ctrl == (HDLC_UI_CMD | HDLC_POLL_FLAG)))
		{
		  // process only frame where destination address matches own address
			hdlc.rx_frame_fcs = (uint16_t)(buf[len-2]<<8) | (uint16_t)(buf[len-1]);
			if (len>5)
			{  // copy payload
				memcpy(hdlc.p_payload,hdlc.p_rx_frame+3,len-5);
			}
			#ifndef __SKIPCRC__
			if (crc16(buf, len-2) == hdlc.rx_frame_fcs)
			#endif
			{
				// process received payload
				len = payload_processor(&hdlc);
				if (len > 0)
				{
					hdlc_tx_frame(hdlc.p_payload, len);
				}
			}
		}		
	}
}

//// calculate crc16 CCITT
//uint16_t crc16(const uint8_t *data_p, uint8_t length)
//{
//	uint8_t x;
//	uint16_t crc = 0xFFFF;

//	while (length--){
//			x = crc >> 8 ^ *data_p++;
//			x ^= x>>4;
//			crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
//	}
//	return crc;
//}

//uint16_t crc16(const uint8_t *data_p, uint8_t length)
//{
//	#define POLY 0x8408
//      uint8_t i;
//      uint16_t data;
//      uint16_t crc = 0xffff;

//      if (length == 0)
//            return (~crc);

//      do
//      {
//            for (i=0, data=(unsigned int)0xff & *data_p++;
//                 i < 8; 
//                 i++, data >>= 1)
//            {
//                  if ((crc & 0x0001) ^ (data & 0x0001))
//                        crc = (crc >> 1) ^ POLY;
//                  else  crc >>= 1;
//            }
//      } while (--length);

//      crc = ~crc;
//      data = crc;
//      crc = (crc << 8) | (data >> 8 & 0xff);

//      return (crc);
//}



// Transmit HDLC UI frame 
void hdlc_tx_frame(const uint8_t *txbuffer, uint8_t len)
{
	//uint8_t  byte;
	uint16_t crc, i;

	// Prepare Tx buffer
	hdlc.p_tx_frame[0] = hdlc.own_addr;
	hdlc.p_tx_frame[1] = hdlc.src_addr;
	hdlc.p_tx_frame[2] = HDLC_UI_CMD | HDLC_FINAL_FLAG;
	
	for (i=0; i<len; i++)
	{
		hdlc.p_tx_frame[3+i] = *txbuffer++;
	}
	
	// Calculate CRC
	crc = crc16(hdlc.p_tx_frame, len+3);
	
	// Send/escaped buffer
	for (i=0; i<len+3; i++)
	{
		hdlc_esc_tx_byte(hdlc.p_tx_frame[i]);		// Send byte with esc checking
	}
		
	hdlc_esc_tx_byte((uint8_t)((crc>>8)&0xff));  // Send CRC MSB with esc check
	hdlc_esc_tx_byte((uint8_t)(crc&0xff));      // Send CRC LSB with esc check
	hdlc_tx_byte(HDLC_FLAG_SOF); 		// Send flag - stop frame
}


// Transmit "RAW" HDLC UI frame --- just added SOF and CRC
void hdlc_tx_raw_frame(const uint8_t *txbuffer, uint8_t len)
{
	uint8_t  byte;
	uint16_t crc = crc16(txbuffer, len);
	
	hdlc_tx_byte(HDLC_FLAG_SOF); 			// Send flag - indicate start of frame
	while(len)
	{
		byte = *txbuffer++; 	// Get next byte from buffer
		hdlc_esc_tx_byte(byte);		// Send byte with esc checking
		len--;
	}
	
	hdlc_esc_tx_byte((uint8_t)((crc>>8)&0xff));  // Send CRC MSB with esc check
	hdlc_esc_tx_byte((uint8_t)(crc&0xff));      // Send CRC LSB with esc check
	hdlc_tx_byte(HDLC_FLAG_SOF); 		// Send flag - stop frame
}



/* Copyright (c) 2016 S54MTB			********* End Of File   ********/
