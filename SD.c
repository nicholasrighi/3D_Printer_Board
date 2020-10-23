#include "SD.h"
#include "USART.h"
#include "SPI.h"
#include "global_macros.h"
#include "io_macros.h"


#include <util/delay.h>

//SD card commands
#define CMD0	0
#define CMD1	1
#define CMD8	8
#define CMD9	9
#define CMD10	10
#define CMD12	12
#define CMD16	16
#define CMD17	17
#define CMD41	41
#define AMCD41	41
#define	CMD55	55
#define CMD58	58


//command related data
#define DUMMY_BYTE		0xFF
#define CMD0_CRC		0x95
#define CMD8_CRC		0x87
#define CMD8_ARG		0x000001AA
#define ACMD41_ARG		0x40000000
#define HEADER			0x40
#define FOOTER			1
#define MSG_LENGTH		6
#define RESET_ACK		0
#define	MAX_RSP_BYTES	8
#define BLOCK_SIZE		512
#define CMD17_TOKEN		0xFE
#define MAX_R_ATTEMPTS	2000

//responses from SD card
#define VALID_R1		1
#define NO_ERROR		0

//macros for CS pin
#define DDR_SPI			DDRB
#define PORT_SPI		PORTB
#define CS				PB0

#define CS_LOW()		PORT_SPI &= ~(1 << CS)
#define CS_HIGH()		PORT_SPI |= (1 << CS)

#define SD_USART		USART0	

/*
	function:		send_SD_command
	
	description:	formats the SD command and sends it to the SD card. Does not send
 					CRC data except for on CMD0 and CM8, for which hard coded 
					CRC values are used. This function drives CS high, sends a dummy
					byte, then drives CS low, then send the specified command. This function
					does now drive CS high again after the command is sent, since CS must
					be low while responses are being sent. 
					
	params:			command		-command to send to SD card. Must be a macro listed in 
								 SD.h under "SPI commands for SD cards"
					
					data		-data to send with command. If command doesn't need 
								 data, any value can be passed as a parameter
	
	return:			void
*/
void send_SD_command(uint8_t command, uint32_t data);



/*
	function:		get_SD_response
	
	description:	sends out dummy values on the MOSI line at most MAX_RSP_BYTES times. If any valid data
					is on the MISO line during this time, the function stops sending data and returns the 
					data on the MISO line to the caller. Valid data is defined as any data that isn't 0xFF.
	
	params:			none
	
	return:			byte currently on the MISO line
*/
uint8_t get_SD_response();



/*
	function:		MISO_to_buffer
	
	description:	reads data from the MISO line into a buffer. Sends out dummy bytes (OxFF) while doing so,
					which means both the MISO and MOSI lines will be busy during this time
	
	params:			buffer		-buffer to write data to, must already be allocated
	
					length		-number of bytes to write to the buffer (maximum number
								of bytes to write is 2^16 bytes)
					
	return:			none
*/
void MISO_to_buffer(uint8_t* buffer, uint16_t length);



//see http://elm-chan.org/docs/mmc/i/sdinit.png for full initialization flow
void setup_SD_card() {
	
	#ifdef DEBUG
	USART_init(SD_USART, 14400, ASYNCH_USART, NO_PARITY, ONE_STOP_BIT, EIGHT_BIT);
	#endif
	
	//enable pull up on CS before setting it as an output to prevent SPI from going into slave mode
	CS_HIGH();
	
	//set CS as output
	DDR_SPI |= (1 << CS);
	
	init_SPI(SPI_MODE0, SPI_PRESCALE_128, SPI_MASTER, MSB_FIRST, DISABLE_2X);
	
	//wait for SD card to startup before sending any data
	_delay_ms(2);
	
	//send dummy bytes to allow SD card to read clk signal
	//CS must be high during this period
	for (int i = 0; i < 80; i++) {
			SPI_send_recieve(DUMMY_BYTE);
	}
	
	//reset card with CMD0
	send_SD_command(CMD0, 0);
		
	uint8_t response = get_SD_response();
	
	#ifdef DEBUG
		USART_transmit_str(SD_USART, "CMD0 response: ");
		USART_hex2char(SD_USART, response, ADD_NEWLINE);
	#endif
	
	//determine card type with CM8
	send_SD_command(CMD8, CMD8_ARG);
	
	response = get_SD_response();
	
	#ifdef DEBUG
		USART_transmit_str(SD_USART, "CMD8 Response: ");
		USART_hex2char(SD_USART, response, ADD_NEWLINE);
	#endif
	
	uint8_t cmd8_response[4];

	//CMD8 is valid, read in rest of response
	if (response != DUMMY_BYTE) {
		MISO_to_buffer(cmd8_response, 4);
	}
	
	#ifdef DEBUG
		for (int i = 0; i < 3; i++) {
			USART_hex2char(SD_USART, cmd8_response[i], NO_NEWLINE);
		}
		USART_hex2char(SD_USART, cmd8_response[3], ADD_NEWLINE);
	#endif
	
	//keep sending command until card is in idle state
	do {
		
		send_SD_command(CMD55, 0);
		
		response = get_SD_response();
		
		send_SD_command(AMCD41, ACMD41_ARG);
		
		response = get_SD_response();
		
		_delay_ms(50);
		
	} while (response != 0x00);
	
	send_SD_command(CMD58, 0);
	
	response = get_SD_response();
	
	#ifdef DEBUG
		USART_transmit_str(SD_USART, "CMD58 Response: ");
		USART_hex2char(SD_USART, response, ADD_NEWLINE);
	#endif
	
	MISO_to_buffer(cmd8_response, 4);
	
	#ifdef DEBUG
		for (int i = 0; i < 3; i++) {
			USART_hex2char(SD_USART, cmd8_response[i], NO_NEWLINE);
		}
			USART_hex2char(SD_USART, cmd8_response[3], ADD_NEWLINE);
	#endif
}



void send_SD_command(uint8_t command, uint32_t data) {
	
	//drive chip select low and allow for synchronization
	CS_HIGH();
	SPI_send_recieve(DUMMY_BYTE);
	CS_LOW();
	SPI_send_recieve(DUMMY_BYTE);
	
	//comm_packet[0] corresponds to LSB, comm_packet[5]
	//corresponds to MSB
	uint8_t comm_packet[MSG_LENGTH];
	
	comm_packet[5] = HEADER | command;
	comm_packet[4] = (data >> 24);
	comm_packet[3] = (data >> 16);
	comm_packet[2] = (data >> 8);
	comm_packet[1] = data;
	
	if (command == CMD0) {
		comm_packet[0] = CMD0_CRC;
	} else if (command == CMD8) {
		comm_packet[0] = CMD8_CRC;
	} else {
		comm_packet[0] = 0;
	}
	
	//send all bytes, MSByte first
	for (int i = MSG_LENGTH - 1; i >= 0; i--) {
		SPI_send_recieve(comm_packet[i]);
	}
	
}



uint8_t get_SD_response() {
	
	uint8_t index = 0;
	uint8_t response;
	
	do {
		
		response = SPI_send_recieve(DUMMY_BYTE);
		index++;
		
	} while(response == DUMMY_BYTE && index < MAX_RSP_BYTES);
	
	return response;
}



void MISO_to_buffer(uint8_t* buffer, uint16_t length) {
	
	for (int i = 0; i < length; i++) {
		buffer[i] = SPI_send_recieve(DUMMY_BYTE);	
	}
}


uint8_t read_SD_card(uint8_t* buffer, uint32_t LBA_addr) {
	
	send_SD_command(CMD17, LBA_addr);
	
	uint8_t response = get_SD_response();
	
	if (response != NO_ERROR) {
		#ifdef DEBUG
			USART_transmit_str(SD_USART, "CMD17 resulted in illegal response: ");
			USART_hex2char(SD_USART, response, ADD_NEWLINE);
		#endif
		return ERROR;
	}
	
	uint16_t current_read_attempts = 0;
	
	//wait until valid response token is received
	do {
		response = SPI_send_recieve(DUMMY_BYTE);
	} while (current_read_attempts < MAX_R_ATTEMPTS && response != CMD17_TOKEN);
	
	for (uint16_t i = 0; i < 512; i++) {
		buffer[i] = SPI_send_recieve(DUMMY_BYTE);
	}
	
	return NO_ERROR;
}
