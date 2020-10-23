#include "global_macros.h"
#include "io_macros.h"
#include "Timers.h"

#define DISABLE_OVERFLOW		0xFE									//Mask for disabling overflow interrupt
#define DISABLE_A0				~(1 << COM0A0 | 1 << COM0A1)			//Mask for disabling OC0A pin's pwm output
#define DISABLE_B0				~(1 << COM0B0 | 1 << COM0B1)			//Mask for disabling OC0B pin's pwm output
#define DISABLE_C1				~(1 << COM1C1 | 1 << COM1C0)			//Mask for disabling OC1C pin's pwm output
	
#define CLEAR_PWM				~(1 << WGM00 | 1 << WGM01)				//Mask for disabling pwm on timer0
						
#define EIGHT_BIT_TOP			255.0f									//max value for timer0

#define CLEAR_TIMER0_PRESCALER	~(1 << CS02 | 1 << CS01 | 1 << CS00)	//Mask for clearing timer0 clock bits
#define CLEAR_TIMER1_PRESCALER	~(1 << CS12 | 1 << CS11 | 1 << CS10)	//Mask for clearing timer1 clock bits

/*
	function:		set_timer16_prescaler
	
	description:	Sets the clock speed the timer to F_CPU/prescaler. This also resets the counter for timer1 back to 0. 
					This will effect all functions relying on the specified timer, so be careful about changing 
					the prescaler while other functions are using the timer. It's recommended to either call this 
					function during setup or only call the function while the timer is stopped. 
	
	parameters:		timer			-timer whose prescaler value is being changed. Must be TIMER1 or TIMER3
	
					prescaler		-Must be a valid prescaler macro from this header file
	
	return:			void
*/
void set_timer16_prescaler(uint8_t timer, uint8_t prescaler);

void init_pwm_timer0(uint8_t port, uint8_t pwm_mode, uint8_t prescaler, uint8_t set_cond, float duty_cycle) {
	
	//start clock at 0
	TCNT0 = 0;

	TCCR0A &= CLEAR_PWM;
	TCCR0A |= pwm_mode;

	//setup condition for setting/clearing output port
	if (port == A0) {
		TCCR0A &= DISABLE_A0;
		TCCR0A |= (set_cond << COM0A0);
	} else if (port == B0) {
		TCCR0A &= DISABLE_B0;
		TCCR0A |= (set_cond << COM0B0);
	}
	
	//setup duty cycle
	if (port == A0) {
		OCR0A = duty_cycle * EIGHT_BIT_TOP;
		SET_PIN_OUTPUT(B, 7);
	} else if (port == B0) {
		OCR0B = duty_cycle * EIGHT_BIT_TOP;
		DDRD |= (1 << DDD5);
		SET_PIN_OUTPUT(G, 5);
	}
	
}

void disable_pwm_timer0(uint8_t port) {
	if (port == A0) {
		TCCR0A &= DISABLE_A0;
	} else if (port == B0) {
		TCCR0A &= DISABLE_B0;
	}
}



void enable_timer0_overflow_interrupt() {
	TIMSK0 |= (1 <<TOIE0);
}



void disable_timer0_overflow_interrupt() {
	TIMSK0 &= DISABLE_OVERFLOW;
}



void stop_timer0_clock() {
	set_timer0_prescaler(CLOCK_OFF);
}



void set_timer0_prescaler(uint8_t prescaler) {
	
	TCNT0 = 0;
	
	//turn of timer0 clock
	TCCR0B &= CLEAR_TIMER0_PRESCALER;
	
	TCCR0B |= prescaler;
	
}



void init_pwm_timer16(uint8_t port, uint32_t frequency, uint8_t prescaler, float duty_cycle) {
	
	//using pointers allows for this function to modify both timers 1 and 3
	//without redundant code
	volatile uint8_t* regA = 0;
    volatile uint8_t* regB = 0;
	
	uint8_t timer = -1;
	
	if (port == A1 || port == B1 || port == C1) {
		regA = &TCCR1A;
		regB = &TCCR1B;
		timer = TIMER1;
	} else if (port == A3 || port == B3 || port == C3) {
		regA = &TCCR3A;
		regB = &TCCR3B;
		timer = TIMER3;
	}
	
	//setup fast PWM
	*regA |= (1 << WGM11);
	*regB |= ((1 << WGM13) | (1 << WGM12));
	
	
	uint16_t true_prescaler = -1;
	
	//convert prescaler macro to numerical value for frequency calculation
	switch(prescaler) {
		case TIMER16_PRESCALE_1:
				true_prescaler = 1;
				break;
		case TIMER16_PRESCALE_8:
				true_prescaler = 8;
				break;
		case TIMER16_PRESCALE_64:
				true_prescaler = 64;
				break;
		case TIMER16_PRESCALE_256:
				true_prescaler = 256;
				break;
		case TIMER16_PRESCALE_1024:
				true_prescaler = 1024;
				break;
		default:
				break;
	}
	
	//set frequency using frequency = (F_CPU) / (prescaler * (1 + TOP))
	if (port == A1 || port == B1 || port == C1) {
		ICR1 = (F_CPU/(uint32_t)(true_prescaler * frequency)) - 1;
	} else if (port == A3 || port == B3 || port == C3) {
		ICR3 = (F_CPU/(uint32_t)(true_prescaler * frequency)) - 1;
	}
	
	//configure set and clear conditions for fast pwm
	//configure duty cycle and set output pins
	switch(port) {
		case A1:
			*regA &= DISABLE_A0;
			*regA |= (1 << COM1A1);
			OCR1A = ICR1 * duty_cycle;
			SET_PIN_OUTPUT(B, 5);
			break;
		case B1:
			*regA &= DISABLE_B0;
			*regA |= (1 << COM1B1);
			OCR1B = ICR1 * duty_cycle;
			SET_PIN_OUTPUT(B, 6);
			break;
		case C1:
			*regA &= DISABLE_C1;
			*regA |= (1 << COM1C1);
			OCR1C = ICR1 * duty_cycle;
			SET_PIN_OUTPUT(B, 7);
			break;
		case A3:
			*regA &= DISABLE_A0;
			*regA |= (1 << COM3A1);
			OCR3A = ICR3 * duty_cycle;
			SET_PIN_OUTPUT(E, 3);
			break;
		case B3:
			*regA &= DISABLE_B0;
			*regA |= (1 << COM3B1);
			OCR3B = ICR3 * duty_cycle;
			SET_PIN_OUTPUT(E, 4);
			break;
		case C3:
			*regA &= DISABLE_C1;
			*regA |= (1 << COM3C1);
			OCR3C = ICR3 * duty_cycle;
			SET_PIN_OUTPUT(E, 5);
			break;
		default:	
			break;
	}
	
	//set timer prescaler last so timer only starts once all other settings are configured
	set_timer16_prescaler(timer, prescaler);
}

void disable_pwm_timer16(uint8_t port) {
	
	uint8_t regA = 0;
	
	if (port == A1 || port == B1 || port == C1) {
		regA = TCCR1A;
	} else if (port == A3 || port == B3 || port == C3) {
		regA = TCCR3A;
	}
	
	switch(port) {
		case A1:
			regA &= DISABLE_A0;
			break;
		case B1:
			regA &= DISABLE_B0;
			break;
		case C1:
			regA &= DISABLE_C1;
			break;
		case A3:
			regA &= DISABLE_A0;
			break;
		case B3:
			regA &= DISABLE_B0;
			break;
		case C3:
			regA &= DISABLE_C1;
			break;
		default:
			break;
	}
	
}


void stop_timer16_clock(uint8_t timer) {
	set_timer16_prescaler(timer, CLOCK_OFF);
}


void set_timer16_prescaler(uint8_t timer, uint8_t prescaler) {
	
	if (timer == TIMER1) {
		TCNT1 = 0;
		
		TCCR1B  &= CLEAR_TIMER1_PRESCALER;
		TCCR1B |= prescaler;
		
	} else if (timer == TIMER3) {
		TCNT3 = 0;
		
		TCCR3B  &= CLEAR_TIMER1_PRESCALER;
		TCCR3B |= prescaler;
	}
}



void enable_timer16_overflow_interrupt(uint8_t timer) {
	
	if (timer == TIMER1) {
		TIMSK1 |= (1 <<TOIE1);
	} else if (timer == TIMER3) {
		TIMSK3 |= (1 <<TOIE3);
	}
	
}



void disable_timer16_overflow_interrupt(uint8_t timer) {
	if (timer == TIMER1) {
		TIMSK1 &= DISABLE_OVERFLOW;
	} else if (timer == TIMER3) {
		TIMSK3 &= DISABLE_OVERFLOW;
	}
}



void set_timer16_frequency(uint8_t timer, uint16_t frequency, uint8_t prescaler) {
		
	//clock is stopped to allow frequency and prescaler values to
	//take effect at the same time
	stop_timer16_clock(timer);	
		
	uint16_t true_prescaler = -1;
	
	//convert prescaler macro to decimal value for frequency calculation
	switch(prescaler) {
		case TIMER16_PRESCALE_1:
			true_prescaler = 1;
			break;
		case TIMER16_PRESCALE_8:
			true_prescaler = 8;
			break;
		case TIMER16_PRESCALE_64:
			true_prescaler = 64;
			break;
		case TIMER16_PRESCALE_256:
			true_prescaler = 256;
			break;
		case TIMER16_PRESCALE_1024:
			true_prescaler = 1024;
			break;
		default:
			true_prescaler = -1;
	}
	
	//set frequency using frequency = ((F_CPU) / (prescaler * (1 + TOP))) - 1
	if (timer == TIMER1) {
		ICR1 = (F_CPU/(uint32_t)(true_prescaler * frequency)) - 1;
	} else if (timer == TIMER3) {
		ICR3 = (F_CPU/(uint32_t)(true_prescaler * frequency)) - 1;
	}
	
	//start clock at the end to prevent output from changing while settings are updated
	set_timer16_prescaler(timer, prescaler);
}