#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "USART.h"


#define HEX2STRING_LENGTH	4

/*
	function:		USART_wait_for_Tx_complete
	
	description:	blocks until all bytes from buffer have been put onto the Tx line,
						then returns. Clears the transmit complete bit
						
	params:			channel		-channel to block on. Must be either USART0 or USART1
	
	return:			void
*/
void USART_wait_for_Tx_complete(uint8_t channel);



void USART_init(uint8_t channel, uint16_t baud_rate, uint8_t op_mode, uint8_t parity, 
				uint8_t stop_bits, uint8_t data_size) {
	
	uint16_t baud_scaled = (((F_CPU / (baud_rate * 16UL))) - 1);
	
	if (channel == USART1) {
		//set baud rate
		UBRR1H = (unsigned char)(baud_scaled>>8);
		UBRR1L = (unsigned char) baud_scaled;
		
		//Enable receiver and transmitter
		UCSR1B = (1<<RXEN0)|(1<<TXEN0);

		//set USART to data format to specified values
		UCSR1C |= (op_mode | parity | stop_bits | data_size);
	} else if (channel == USART0) {
		//set baud rate
		UBRR0H = (unsigned char)(baud_scaled>>8);
		UBRR0L = (unsigned char) baud_scaled;
		
		//Enable receiver and transmitter
		UCSR0B = (1<<RXEN0)|(1<<TXEN0);

		//set USART to data format to specified values
		UCSR0C |= (op_mode | parity | stop_bits | data_size);
	}
	
}

void USART_transmit(uint8_t channel, uint8_t data, uint8_t wait_for_tx_complete) {
	
	if (channel == USART0) {
		/* Wait for empty transmit buffer */
		while (!(UCSR0A & (1<<UDRE0)))
		;
		/* Put data into buffer, sends the data */
		UDR0 = data;
	} else if (channel == USART1) {
		/* Wait for empty transmit buffer */
		while (!(UCSR1A & (1<<UDRE1)))
		;
		/* Put data into buffer, sends the data */
		UDR1 = data;	
	}
	
	if (wait_for_tx_complete == TRUE) {
		USART_wait_for_Tx_complete(channel);
	}
	
}

uint8_t USART_receive(uint8_t channel) {
	
	if (channel == USART0) {
		/* Wait for data to be received */
		while (!(UCSR0A & (1<<RXC0))) {
			
		}
		/* Get and return received data from buffer */
		return UDR0;
	} else {
		/* Wait for data to be received */
		while (!(UCSR1A & (1<<RXC1))) {
			
		}
		/* Get and return received data from buffer */
		return UDR1;
	}
	
}

void USART_enable_Tx_interrupt(uint8_t channel) {
	
	if (channel == USART0) {
		UCSR0B |= (1 << TXCIE0);
	} else if (channel == USART1) {
		UCSR1B |= (1 << TXCIE1);
	}
	
}


void USART_read_to_buffer(uint8_t channel, uint8_t* buffer, uint8_t num_bytes) {
	
	for (uint8_t i = 0; i < num_bytes; i++) {
		buffer[i] = USART_receive(channel);
	}
		
}

void USART_transmit_buffer(uint8_t channel, uint8_t* buffer, uint8_t num_bytes, uint8_t wait_for_Tx_complete) {
	
	//send all but the last byte
	for (int i = 0; i < num_bytes - 1; i++) {
		USART_transmit(channel, buffer[i], FALSE);
	}
	
	//only need to wait on the final byte being sent, since 
	//TXCn is only set when data in the Transmit shift register has been 
	//shifted out and there's no data in UDR0
	//This condition will only be met during the final byte, since every other byte
	//will have a byte in UDR0 waiting to be sent while the current byte is put 
	//onto the bus
	USART_transmit(channel, buffer[num_bytes - 1], wait_for_Tx_complete);
	
}

void USART_wait_for_Tx_complete(uint8_t channel) {
	
	if (channel == USART0) {
		//blocks until all data has been sent out on the Tx line
		while (!(UCSR0A & (1 << TXC0))) {
		}
		
		//clears transmit complete bit by writing a one to TXCO's location
		//this is somewhat counter-intuitive, because setting a bit isn't usually
		//associated with clearing it
		UCSR0A |= (1 << TXC0);
	} else if (channel == USART1) {
		//blocks until all data has been sent out on the Tx line
		while (!(UCSR1A & (1 << TXC1))) {
		}
	
		//clears transmit complete bit by writing a one to TXCO's location
		//this is somewhat counter-intuitive, because setting a bit isn't usually
		//associated with clearing it
		UCSR1A |= (1 << TXC1);
	}
}


void USART_hex2char(uint8_t channel, uint8_t value, uint8_t newline) {

	//string is of format 0xYZ\n, where Y and Z are hex characters
	//this gives a maximum size of 5
	uint8_t ascii_string[HEX2STRING_LENGTH];

	sprintf((char*)ascii_string, "%02X", value);
	
	ascii_string[HEX2STRING_LENGTH - 1] = ((newline == ADD_NEWLINE) ? '\n' : 0);
	
	uint8_t length = ((newline == ADD_NEWLINE) ? HEX2STRING_LENGTH : HEX2STRING_LENGTH - 1);
	
	USART_transmit_buffer(channel, ascii_string, length, FALSE);
}



void USART_transmit_str(uint8_t channel, char* string) {
	
	uint8_t length = strlen(string);
	
	USART_transmit_buffer(channel, (uint8_t *)string, length, FALSE);
	
}