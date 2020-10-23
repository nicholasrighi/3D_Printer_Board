#ifndef IO_MACROS 
#define	IO_MACROS
	
#include <avr/sfr_defs.h>				
					
#define SET_PIN_OUTPUT(p, bit)		(DDR ## p |= _BV(bit))
#define SET_PIN_INPUT(p, bit)		(DDR ## p &= (~(_BV(bit))))

#define SET_PIN_HIGH(p, bit)		(PORT ## p |= _BV(bit))
#define SET_PIN_LOW(p, bit)			(PORT ## p &= (~(_BV(bit))))

#endif