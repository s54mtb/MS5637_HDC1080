/**
  ******************************************************************************
  * File Name          : hdlc.h
  * Description        : HDLC implementation
  ******************************************************************************
  *
  * Copyright (c) 2016 S54MTB
  * Licensed under Apache License 2.0 
  * http://www.apache.org/licenses/LICENSE-2.0.html
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#ifndef __HDLC_H__
#define __HDLC_H__

#define HDLC_MRU    256
// HDLC constants --- RFC 1662 
#define HDLC_FLAG_SOF				  0x7e   // Flag
#define HDLC_CONTROL_ESCAPE 	0x7d   // Control Escape octet
#define HDLC_ESCAPE_BIT     	0x20   // Transparency modifier octet (XOR bit)
#define HDLC_CRC_INIT_VAL   	0xffff
#define HDLC_CRC_MAGIC_VAL   	0xf0b8
#define HDLC_CRC_POLYNOMIAL  	0x8408
#define HDLC_UI_CMD						0x03     // Unnumbered Information with payload
#define HDLC_FINAL_FLAG       0x10     // F flag
#define HDLC_POLL_FLAG       0x10      // P flag

typedef enum
{
	HDLC_SOF_WAIT,
	HDLC_DATARX,
	HDLC_PROC_ESC,
} hdlc_state_t;

typedef struct 
{
	uint8_t				own_addr;
	uint8_t				src_addr;
	uint8_t  			dest_addr;
	uint8_t 			ctrl;
	uint8_t				*p_tx_frame;			// tx frame buffer
	uint8_t 			*p_rx_frame;			// rx frame buffer
	uint8_t				*p_payload;				// payload pointer
	uint16_t   		rx_frame_index;
	uint16_t			rx_frame_fcs;
	hdlc_state_t	state;
} hdlc_t;

void hdlc_init(void);
void hdlc_process_rx_byte(uint8_t rx_byte);
void hdlc_process_rx_frame(uint8_t *buf, uint16_t len);
void hdlc_tx_frame(const uint8_t *txbuffer, uint8_t len);
void hdlc_tx_raw_frame(const uint8_t *txbuffer, uint8_t len);


#endif
