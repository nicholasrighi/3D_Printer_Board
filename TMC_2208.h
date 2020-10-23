#ifndef TMC_2208
#define TMC_2208

#include <stdint-gcc.h>
#include "global_macros.h"

/*	
	micro step resolutions (steps per full step)
	only valid when mstep_reg_select in GCONF is set to 1 (otherwise MS1 and MS2 pins
	determine micro step resolution)
*/
#define MICROSTEP_NONE					0x8				
#define MICROSTEP_2						0x7				
#define MICROSTEP_4						0x6				
#define MICROSTEP_8						0x5
#define MICROSTEP_16					0x4
#define MICROSTEP_32					0x3
#define MICROSTEP_64					0x2
#define MICROSTEP_128					0x1
#define MICROSTEP_256					0x0


//macros that determine motor parameters
#define MAX_MICROSTEP_PER_FULL_STEP		256					//number of micro steps that make up a full step (motor dependent)
#define MM_PER_FULL_ROTATION			8					//linear distance traveled by axis per 360 degree rotation of motor (axis dependent)
#define FULL_STEPS_PER_ROTATION			200					//how many full steps to move 360 degrees
#define CURRENT_MICROSTEP_VALUE			MICROSTEP_8			//micro step setting for TMC_2208, determines number of micro steps moved per step pulse 
															//a higher micro stepping value (ie. more micro steps per full step) leads to much lower
															//motor torque, which can cause stalling. 

#define TMC_READ_LENGTH					0x4					//number of bytes sent during a read request


/*
	function:		TMC_2208_write
	
	description:	Writes supplied data to the specified register of the TMC_2208. Register must be writable. TMC_2208
					uses the same line for Tx and Rx, so ensure TMC_2208 isn't writing to the data line when 
					attempting to send data. This function blocks until all data has been transmitted over the 
					Tx line. This is done to ensure that the settings being written to the motor can take effect 
					before the motor is controlled. 
	
	parameters:		register_addr		-the address of the register to write to
	
					data				-Data to write to the specified address
					
	return			void
*/
void TMC_2208_write(uint8_t register_addr, uint32_t data);


/*
	function:		TMC_2208_read
	
	description:	Reads the contents from the specified TMC_2208 register into the provided buffer. The buffer
					must be at least 8 bytes long, and the message is stored with the first received
					byte from the TMC_2208 in the lowest memory address (see diagram below). This function disables 
					USART receive before sending a read request to the TMC_2208 and re-enables USART receive
					once the function completes. This means the receive buffer will be cleared once this function
					is called, so any needed data must be read out of the buffer first. This also means that 
					USART receive interrupts won't trigger while this function is running, since the receive bit
					isn't set. 
						
						-----------------------------------------------------
						|First received byte	|....|Last received byte	| (buffer)
						-----------------------------------------------------
						[0].....					  [7]					  (buffer addresses)
	
	parameters:		buffer				-buffer to store TMC_2208 response
	
					register_addr		-address of register to read from
	
	return:			void 
*/
void TMC_2208_read(uint8_t* buffer, uint8_t register_addr);



/*
	function:		TMC_2208_setup
	
	description:	Sets the GCONF, CHOPCONF, SLAVECONF, and IHOLD_RUN registers to values defined in TMC_2208.c. These
					settings enable UART control and micro stepping value. See the TMC_2208 data sheet to determine 
					exactly what settings are enabled with this command.
	
	parameter:		none
	
	return:			void
*/
void TMC_2208_setup();


/*
	function:		TMC_2208_get_errors
	
	description:	Returns the contents of the DRV_STATUS register, which contain any errors the TMC_2208 experienced.
					This process blocks until all data has arrived in the buffer
	
	parameters:		buffer		-pointer to an array of size TMC_READ_LENGTH
	
	return:			void
*/
void TMC_2208_get_errors(uint8_t* buffer);
#endif