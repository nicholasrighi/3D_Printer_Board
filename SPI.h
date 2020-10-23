#ifndef SPI
#define SPI

#include <avr/io.h>

#define SPI_MODE0			0x0
#define SPI_MODE1			(1 << CPHA)
#define SPI_MODE2			(1 << CPOL)
#define SPI_MODE3			(1 << CPOL) | (1 << CPHA)

#define SPI_PRESCALE_4		0x0
#define SPI_PRESCALE_16		(1 << SPR0)
#define SPI_PRESCALE_64		(1 << SPR1)
#define SPI_PRESCALE_128	(1 << SPR1) | (1 << SPR0)

#define SPI_MASTER			(1 << MSTR)
#define SPI_SLAVE			0x0

#define MSB_FIRST			0x0
#define LSB_FIRST			(1 << DORD)

#define ENABLE_2X			(1 << SPI2X)
#define DISABLE_2X			0x0
/*
	function:		init_SPI
		
	description:	Sets up the SPI functionality and configures that parameters according to user specifications. This 
					overrides the functionality of the MISO/MOSI/SCK pins, so these pins can't be used by the user
					for other functions. This doesn't use the dedicated SS pin, and the SS pin must be configured as an
					output before calling this function. Failure to set the SS pin as an output can result in the ATmega
					going into SPI slave mode if the SS pin goes low. See the data sheet for full details.
	
	params:			spi_mode		-indicates the idle value of SCK and when data is sampled/setup. Only SPI_MODE macros
									 are valid inputs
									 
					prescaler		-prescaler to divide the clock by. SCK will be set to F_CPU/prescaler
					
					master_slave	-determines if SPI is configured for master of slave mode. Only MASTER_SPI and 
									 slave_SPI are valid inputs
									 
					data_order		-determines if data is sent LSB or MSB first. Only MSB_FIRST and LSB_FIRST are 
									 valid macros
					
					double_speed	-doubles frequency of SCK, so SCK frequency will be set to 2*F_CPU/prescaler.
									 Argument must be either ENABLE_2X or DISABLE_2X
	
	return:			void
*/
void init_SPI(uint8_t spi_mode, uint8_t prescaler, uint8_t master_slave, uint8_t data_order, uint8_t double_speed);


/*
	function:		SPI_send_recieve
	
	description:	Sends data out on MOSI line and returns data on the MISO line
	
	params:			data			-the data to send out on the SPI line
	
	return:			returns the data present on the MISO line 
*/
uint8_t SPI_send_recieve(uint8_t data);

#endif