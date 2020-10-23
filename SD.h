#ifndef SD
#define SD

#include <stdint.h>

#define ERROR	-1
#define NO_ERROR 0

/*
	function:		setup_SD_card
	
	description:	puts SD card through initialization process to determine the type of SD card and 
					set the read/write block size to 512 bytes.
					
	params:			none
	
	return:			void
*/
void setup_SD_card();



/*
	function:		read_SD_card
	
	description:	Reads the 512 byte block from the SD card into the specified buffer. This means the buffer must be
					at least 512 bytes long. Returns ERROR if an error occurred while reading, and NO_ERROR if no 
					error occurred while reading. If ERROR is set the data written into the buffer isn't guaranteed to 
					be valid, so you should call this function again to write all data into the buffer.
	
	params:			buffer			-the buffer to write data into. Must be at least 512 bytes long		
		
					LBA_addr		-the LBA address (block number) to read from the SD card
	
	return:			uint8_t			-returns ERROR if an error occurred, returns NO_ERROR if no error occurred
*/
uint8_t read_SD_card(uint8_t* buffer, uint32_t LBA_addr);
#endif