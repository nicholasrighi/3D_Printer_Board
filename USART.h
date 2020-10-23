#ifndef USART
#define USART

#include <avr/io.h>
#include "global_macros.h"

#ifndef F_CPU
/* prevent compiler error by supplying a default */
# warning "F_CPU not defined for \"USART.h\""
/*
	CPU frequency is needed to configure baud rate. Inaccurate CPU macro
	will result in garbage data being transmitted via UART
 */
# define F_CPU			1000000UL
#endif

//USART operating modes
#define ASYNCH_USART	0
#define SYCN_USART		(1 << UMSEL00)

//USART parity
#define EVEN_PARITY		(1 << UPM01)
#define ODD_PARITY		(1 << UPM01) | (1 << UPM00)
#define NO_PARITY		0

//USART check bits
#define ONE_STOP_BIT	0
#define TWO_STOP_BIT	(1 << USBS0)

//USART data sizes (9 bit not supported)
#define FIVE_BIT		0
#define SIX_BIT			(1 << UCSZ00)
#define SEVEN_BIT		(1 << UCSZ01)
#define EIGHT_BIT		(1 << UCSZ01) | (1 << UCSZ00)

//USART channels
#define USART0			1
#define USART1			2

#define ADD_NEWLINE		3
#define NO_NEWLINE		4
/*	
	function:		USART_init
	
	description:	Sets all registers needed to configure USART. Does not enable USART interrupts. Does not support 9-bit data
					mode. The settings chosen should match the settings of target slave device. 
	
	parameters		channel			-the USART channel (USART0 or USART1) to configure
	
					baud_rate		-speed of data transmission. See ATmega328p or ATmega1281 data sheet for common baud rates/associated
										error rate with a given clock source.
										
					op_mode			-Controls if data is sent synchronously or asynchronously. A clock signal must be supplied to slave device 
										if operated asynchronously. Must use ASYCH or SYNCH as arguments.
										
					parity			-Determines if an extra bit is added at the end of the message, for error correction. Must use
										EVEN_PARITY, ODD_PARITY, or NO_PARITY as arguments
					
					stop_bits		-Number of stop bits added to end of message. Must use ONE_STOP_BIT or TWO_STOP_BIT as arguments
					
					data_size		-How many data bits are sent per UART packet. Supplied sizes are 5-8, must use a supplied macro 
										as an argument. 
	return:			void
*/
void USART_init(uint8_t channel, uint16_t baud_rate, uint8_t op_mode, uint8_t parity, uint8_t stop_bits, uint8_t data_size);



/*
	function:		USART_transmit
	
	description:	Blocks until USART send buffer is empty, then sends the specified data out on the Tx line. Data is sent LSB first
	
	parameters:		channel					-USART channel to send data on. Must be USART0 or USART1		
		
					data					-data to send on Tx line. If data supplied is fewer than 8 bits, or chosen size for UART communication
													is fewer than eight bits, the extra bits will be filled with zeros.
					
					wait_for_tx_complete	-flag to indicate if the function should block until all data written to the Tx buffer
												has been sent out on the line. Passing TRUE will block until data is sent, passing false
												won't block.
								
	return:			void
*/
void USART_transmit(uint8_t channel ,uint8_t data, uint8_t wait_for_tx_complete);



/*
	function:		USART_recieve
	
	description:	blocks until USART buffer has received data, then returns the data stored in the buffer
	
	parameters:		channel		-USART channel to read data from. Must be either USART0 or USART1
	
	return:			void
*/
uint8_t USART_receive(uint8_t channel);



/*
	function:		USART_enable_Tx_interrupt
	
	description:	enables the transmit complete interrupt, which triggers when data is sent
					from the USART buffer. This user must create an ISR associated with 
					Tx complete interrupt if they wish to use the interrupt
						
	parameters:		channel		-USART channel to enable interrupts on. Must be either USART0 or USART1
	
	return:			void
*/
void USART_enable_Tx_interrupt(uint8_t channel);



/*
	function:		USART_read_to_buffer
	
	description:	blocks until the byte_length bytes have been read into the buffer. Buffer must already
					be allocated with enough room for incoming data
	
	parameter:		channel		-USART channel to read data from. Must be either USART0 or USART1
	
					buffer		-pointer to an allocated buffer of appropriate size, data
									will be stored in this buffer
									
					num_bytes	-number of bytes that should be read into the buffer
	
	return:			void
*/
void USART_read_to_buffer(uint8_t channel, uint8_t* buffer, uint8_t num_bytes);


/*
	function:		USART_transmit_buffer
	
	description:	blocks until all bytes from buffer have been sent. First byte sent sis
					at address 0, last byte sent is address (num_bytes - 1)
	
	parameters:		channel					-USART channel to send data on. Must be either USART0 or USART1
	
					buffer					-pointer to a buffer of data
					
					num_bytes				-number of bytes to send from specified buffer
					
					wait_for_tx_complete	-flag to indicate if the function should block until all data written to the Tx buffer
												has been sent out on the line. Passing TRUE will block until data is sent, passing false
												won't block.
												
	return:			void
*/
void USART_transmit_buffer(uint8_t channel, uint8_t* buffer, uint8_t num_bytes, uint8_t wait_for_tx_complete);


/*
	function:		USART_put_c
	
	description:	writes the passed int to the specified USART channel as a hex ASCII string. A newline character
					is also written to the USART channel if the user passes ADD_NEWLINE as an argument. This function
					does not wait for all bits to be transmitted before returning, so some data will be written to 
					the Tx line after this function returns.
	
	parameters:		USART_channel		-the channel to write the character to. Must be either 
										 USART0 or USART1
					
					value				-the int to write to the USART line
					
					newline				-writes a newline to the USART channel after the character if ADD_NEWLINE 
										 is specified. No newline is written if NO_NEWLINE is specified. Argument
										 must be ADD_NEWLINE or NO_NEWLINE. 
										
*/
void USART_hex2char(uint8_t channel, uint8_t value, uint8_t newline);


/*
	function:		USART_transmit_str
	
	description:	Writes the string to the specified USART channel. String must be null terminated, and no new line
					character is added to the end of the string. 
					
	parameters:		channel			-the USART channel to write the data to. Must be USART0 or USART1
	
					string			-the string to write to the USART channel

	return:			void
*/
void USART_transmit_str(uint8_t channel, char* string);

#endif
