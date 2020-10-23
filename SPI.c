#include "SPI.h"

#define ENABLE_SPI	(1 << SPE)

#define CLEAR_DOUBLE_SPEED	(~(1 << SPI2X))
#define CLEAR_SPI_INIT		0x80

#define MOSI	DDB2	
#define SCK		DDB1
#define MISO	DDB3

void init_SPI(uint8_t spi_mode, uint8_t prescaler, uint8_t master_slave, uint8_t data_order, uint8_t double_speed) {
	
	//set MOSI, SCK as outputs
	DDRB |= (1 << MOSI) | (1 << SCK);
	
	//enable pull up on MISO
	PORTB |= (1 << MISO);
	
	SPCR &= CLEAR_SPI_INIT;
	SPCR |= (ENABLE_SPI | spi_mode | prescaler | master_slave | data_order);
	
	SPSR &= CLEAR_DOUBLE_SPEED;
	SPSR |= double_speed;
	
}

uint8_t SPI_send_recieve(uint8_t data) {
	
	SPDR = data;
	
	while (!(SPSR & (1 << SPIF))) {
		
	}
	
	return SPDR;
}