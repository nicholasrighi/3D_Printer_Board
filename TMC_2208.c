#include "TMC_2208.h"
#include "USART.h"
#include <stdlib.h>

//message specific bytes
#define SYNC_BYTE			0x5				//first byte sent for every read/write
#define TMC2208_ADDR		0x0				//TMC_2208 device address (all TMC_2208 have addr 0)
#define REG_WRITE_HEADER	0x80			//value added to register_addr for every read request
#define TMC_WRITE_LENGTH	0x8				//number of bytes sent during a write request
#define TMC_REPLY_LENGTH	0x8				//number of bytes sent by TMC_2208 in response to a read request

//TMC2208 registers
#define GCONF				0x0				//global configuration register
#define SLAVECONF			0x3				//register determining how long TMC waits before sending reply after a request received
#define CHOPCONF			0x6C			//configuration register for stealth chop, also holds micro step registers
#define IHOLD_IRUN			0x10			//configuration register for maximum current motor can draw when running/holding in place
#define DRV_STATUS			0x6F			//status register for TMC_2208, shows errors if any are present
#define PWMCONF				0x70			//register for setting stealth chop settings
#define IFCNT				0x02			//8 bit counter that increments with every successful UART write

//TMC2208 register settings
#define GCONF_SETUP			0x1E3			//Determines settings for TMC_2208 on startup, in GCONF register
#define SENDDELAY			0x3				//time that TMC2208 waits until replying to a message, in SLAVECONF register
#define CHOPCONF_DEFAULT	0x10000053UL	//power on default settings, in CHOPCONF
#define IHOLD				0x0				//current through motor while holding still, in IHOLD_IRUN
#define IRUN				0x1F			//maximum current through motor when moving, in IHOLD_IRUN
#define FREEWHEEL			0x1				//determines standstill motor setting
#define IHOLDDELAY			0xA				//number of clock cycles motor takes to power down after standstill detected,
											//in IHOLD_IRUN
//Bit locations in registers
#define IRUN_SHIFT			0x8				//amount to left shift IRUN to get correct settings in IHOLD_IRUN
#define IHOLDDELAY_SHIFT	0x10			//amount to left shift IHOLDDELAY to get correct settings in IHOLD_IRUN
#define SENDDELAY_BIT_SHIFT	0x8				//amount to left shift SENDDELAY to get correct settings in SLAVECONF
#define MRES				0x18			//amount to left shift CURRENT_MICROSTEP_VALUE to get correct settings in CHOPCONF
#define FREEWHEEL_OFFSET	0x14			//amount to left shift FREEWHEEL to get correct settings in PWMCONF 

#define TMC_USART		USART1


/*
	function:		swuart_calcCRC
		
	description:	Helper function that performs the CRC used for TMC_2208 communication (function provided in
						TMC_2208 data sheet). Performs CRC on buffer message, puts CRC into the last byte of the message,
						allows for detection of up to 8 single bit changes.
					
	parameters:		datagram		-pointer to buffer containing message to be transmitted
	
					datagramLength	-number of bytes in message, including the CRC byte this function will add. For
										read write messages the length is 8, for read requests the length is 4.
	
	return:			void
*/
void swuart_calcCRC(unsigned char* datagram, unsigned char datagramLength);



void TMC_2208_write(uint8_t register_addr, uint32_t data) {
	
	uint8_t uart_datagram[TMC_WRITE_LENGTH];
	
	uart_datagram[0] = SYNC_BYTE;
	uart_datagram[1] = TMC2208_ADDR;
	uart_datagram[2] = REG_WRITE_HEADER + register_addr;
	
	//copy each byte from data into uart_datagram
	//data is stored in little endian format (see TMC_2208 datasheet for specifics)
	for (int i = 0; i < 4; i++) {
		uart_datagram[3 + i] = (unsigned char) (data >> ((3 - i) * WORD_SIZE));
	}
		
	//swuart_calcCRC expects the length of the datagram in bytes, which is 8 for writing to a register
	swuart_calcCRC(uart_datagram, TMC_WRITE_LENGTH);
	
	USART_transmit_buffer(TMC_USART, uart_datagram, TMC_WRITE_LENGTH, TRUE);
}



void TMC_2208_read(uint8_t* buffer, uint8_t register_addr) {
	
	//disable receive to prevent reading data that we're sending on the line
	//this is required since the Tx and Rx lines are connected due to the TMC_2208's 
	//serial interface being one wire.
	UCSR0B &= ~(1 << RXEN0);
		
	uint8_t uart_datagram[TMC_READ_LENGTH];
	
	uart_datagram[0] = SYNC_BYTE;
	uart_datagram[1] = TMC2208_ADDR;
	
	//unlike the write datagram, the MSB of the register byte is 0, not 1
	uart_datagram[2] = register_addr;
	
	//swuart_calcCRC expects the length of the datagram in bytes, which is 4 for reading a register
	swuart_calcCRC(uart_datagram, TMC_READ_LENGTH);
	
	USART_transmit_buffer(TMC_USART, uart_datagram, TMC_READ_LENGTH, TRUE);
		
	//re-enable receive 
	UCSR0B |= (1 << RXEN0);	
	
	USART_read_to_buffer(TMC_USART, buffer, TMC_REPLY_LENGTH);
}
void TMC_2208_setup() {		//set global registers on TMC_2208	TMC_2208_write(GCONF, GCONF_SETUP);		//set micro step resolution to selected value	TMC_2208_write(CHOPCONF, CHOPCONF_DEFAULT | ((uint32_t) CURRENT_MICROSTEP_VALUE << MRES));		//set how long TMC_2208 waits before replying to a message	TMC_2208_write(SLAVECONF, (uint32_t)(SENDDELAY << SENDDELAY_BIT_SHIFT));		//set maximum motor current while moving and standing still, as well as time taken to ramp down to standstill	//after moving 	TMC_2208_write(IHOLD_IRUN, ( ((uint32_t)IHOLDDELAY << IHOLDDELAY_SHIFT) | (IRUN << IRUN_SHIFT) | IHOLD) );		//set stealth chop settings	TMC_2208_write(PWMCONF, (uint32_t)FREEWHEEL << FREEWHEEL_OFFSET);		uint8_t buffer[TMC_REPLY_LENGTH];		TMC_2208_read(buffer, IFCNT);}

void TMC_2208_get_errors(uint8_t* buffer) {
	TMC_2208_read(buffer, DRV_STATUS);
}


void swuart_calcCRC(unsigned char* datagram, unsigned char datagramLength)
{
	int i,j;
	unsigned char* crc = datagram + (datagramLength-1); // CRC located in last byte of message
	unsigned char currentByte;
	*crc = 0;
	for (i=0; i<(datagramLength-1); i++) {	// Execute for all bytes of a message
		currentByte = datagram[i]; // Retrieve a byte to be sent from Array
		for (j=0; j<8; j++) {
			if ((*crc >> 7) ^ (currentByte&0x01)) // update CRC based result of XOR operation
			{
				*crc = (*crc << 1) ^ 0x07;
			}
			else
			{
				*crc = (*crc << 1);
			}
			currentByte = currentByte >> 1;
		} // for CRC bit
	} // for message byte
}