#ifndef TIMERS
#define TIMERS

#include <avr/io.h>

//timer0 macros

//8 bit timer ports
#define A0									0x0	//output pin is OC0A
#define B0									0x1	//output pin is OC0B

//PWM modes
#define TIMER0_FAST_PWM						(1 << WGM01) | (1 << WGM00)
#define TIMER0_PHASE_CORR_PWM				(1 << WGM00)

//pre-scalers for clock timer. Timer frequency will be F_CPU/prescaler
#define TIMER0_PRESCALE_1					(1 << CS00)								
#define TIMER0_PRESCALE_8					(1 << CS01)
#define TIMER0_PRESCALE_64					(1 << CS01) | (1 << CS00)
#define TIMER0_PRESCALE_256					(1 << CS02)
#define TIMER0_PRESCALE_1024				(1 << CS02) | (1 << CS00)


//conditions that will set OC0A/OC0B while pwm is running
#define TIMER0_FAST_PWM_NON_INVERTING		0x2
#define TIMER0_FAST_PWM_INVERTING			0x3
#define TIMER0_PHASE_CORR_PWM_CLEAR_UP		0x2
#define TIMER0_PHASE_CORR_PWM_CLEAR_DOWN	0x3

//timer1 and timer3 macros

//16 bit timer ports
#define A1									0x2	//output pin is OC1A
#define B1									0x3	//output pin is OC1B
#define C1									0x4	//output pin is OC1C
#define A3									0x5	//output pin is OC3A
#define B3									0x6	//output pin is OC3B
#define C3									0x7	//output pin is OC3C

#define TIMER1								0x8	
#define TIMER3								0x9

//pre-scalers for clock timer. Timer frequency will be F_CPU/prescaler
#define TIMER16_PRESCALE_1					(1 << CS10)
#define TIMER16_PRESCALE_8					(1 << CS11)
#define TIMER16_PRESCALE_64					(1 << CS11) | (1 << CS10)
#define TIMER16_PRESCALE_256				(1 << CS12)
#define TIMER16_PRESCALE_1024				(1 << CS12) | (1 << CS10) 

#define CLOCK_OFF							0x00		//Mask for disabling timer0 and timer1's clocks


/*
	function:	init_pwm_timer0	

	description:	Sets up the specified port for pwm output with the use specified options enabled (for a full explanation of the different
					pwm options available look at the data sheet for the ATmega328p or ATmega1281). This function starts timer0's clock
					and changes the associated prescaler, so any other functions using timer0 will also be effected (such as timer0 interrupts). 
					Note: this is not the same behavior as init_pwm_timer1, since init_pwm_timer1 does not start the clock associated with
					timer1. 

	params:			port		-the port name for output pin that will be controlled by pwm. Must use either
								A0 and B0 as inputs (which correspond to OC0A and OC0B output pins, respectively)
									
					pwm_mode	-type of pwm (fast or phase corrected) that will be enabled
					
					prescaler	-scales pwm frequency to F_CPU/prescaler. Must use a valid prescaler macro
								as an argument. 
									
					set_cond	-determines when the output pin is set and cleared. Must use a valid set condition
								macro as an argument
									
					duty_cycle	-percentage (expressed as a decimal between 0 and 1) of time that pwm signal is high

	return:			void
*/
void init_pwm_timer0(uint8_t port, uint8_t pwm_mode, uint8_t prescaler, uint8_t set_cond, float duty_cycle);



/*
	function:		disable_pwm_timer0

	description:	disables timer0's pwm signal. This means the pins OC0A or OC0B (depending on port given) wont generate outputs, and can be configured for other 
					uses. This does not stop the timer from running, so the user will need to additionally stop the timer clock if they wish to halt
					other behavior associated with the timer

	params:			the port (A0 or B0) to disable PWM on

	returns:		void					
*/
void disable_pwm_timer0(uint8_t port);



/*
	function:		enable_timer0_overflow_interrupt

	description:	Enables the timer0 overflow interrupt, triggering the associated ISR when the timer overflows. The timer prescaler 
					is also set. Since timer0 can also be used for pwm, setting the interrupt will change the prescaler for any pwm 
					signal on timer0 in addition to setting the interrupt. This function does not set the global interrupt flag, so
					the user must call sei() before the overflow interrupt will take effect
		
	returns:		void
*/
void enable_timer0_overflow_interrupt();



/*
	function:		disable_timer0_overflow_interrupt

	description:	Disables the timer0 interrupt by clearing the timer0 interrupt flag. This does not change the timer0 clock,
					so any pwm signals generated by timer0 will be unchanged. 

	return:			void
*/
void disable_timer0_overflow_interrupt();

/*
/	function:		set_timer0_prescaler();

	description:	Stops timer0 from being incremented by the I/O clock. This will freeze A0 and B0 at their current 
					output value, and will leave them configured as outputs. If disabling those pins is desired, 
					user must also call disable_pwm_timer0 with the appropriate port. This will still leave the 
					pins associated with A0 and B0 as outputs, but at that point the user can use them without 
					disrupting the pwm signals

	return:			void
*/
void stop_timer0_clock();



/*
	function:		set_timer0_prescaler
	
	description:	Sets the prescaler value for timer0's clock. This sets the clock speed for timer1 to F_CPU/prescaler.
					This also resets the counter for timer1 back to 0. This will effect all functions that use timer0,
					so be careful about changing the timer0's prescaler value while running code relying on timer0.
					It's recommended to either call this function during setup or only call the
					function while the timer is stopped.
	
	parameters:		prescaler		-Must be a valid TIMER0_<prescaler value> macro from this header file
	
	return:			void
*/
void set_timer0_prescaler(uint8_t prescaler);



/*
	function:		init_pwm_timer16
	
	description:	Sets the specified ports to fast pwm with the specified frequency and duty cycle. This will effect any 
					functions that rely on the specified timer for timing, so its recommended that all timing related 
					functions have their own dedicated timers. If you want to change the frequency of a timer that is 
					already running, call set_timer16_frequency, which will update the frequency of the specified timer 
					without the overhead of setting the pwm mode and duty cycle. 
					
					Additionally, passing CLOCK_OFF as the prescaler will stop a pwm signal from being generated. This 
					is useful if you want to set up a pwm source and later start it running, since calling
					init_pwm_timer16 has more overhead than set_timer16_frequency.
					
					Note that not all chosen frequencies are possible due to the timer's resolution,
					so the specified and actual frequency can differ. See ATmega1281 data sheet for specifics
	
	parameters:		port			-port to set pwm output on. Must be one of A1, B1, C1, A3, B3, or C3.
	
					frequency		-pwm frequency
					
					prescaler		-prescaler to use for specified frequency, must be a TIMER16_PRESCALE macro or 
									CLOCK_OFF
					
					duty_cycle		-duty cycle of pwm wave, represented as a decimal between 0.0 and 1.0
	
	return:			void

*/
void init_pwm_timer16(uint8_t port, uint32_t frequency, uint8_t prescaler,  float duty_cycle);



/*
	function:		disable_pwm_timer16

	description:	Disables the port's pwm signal. This means the output pin associated with the port wont generate 
					outputs, and can be configured for other uses. This does not stop the timer from running, 
					so the user will need to additionally stop the timer clock if they wish to halt other 
					behavior associated with the timer.

	params:			the port (A1, B1, C1, A3, B3, or C3) to disable PWM on

	returns:		void					
*/
void disable_pwm_timer16(uint8_t port);



/*
	function:		stop_timer16_clock();

	description:	stops the timer from being incremented by the I/O clock. This will freeze any pwm signals at their 
					current value and leave them configured as outputs. If disabling those pins is desired, 
					user must also call disable_pwm_timer16 with the appropriate port. This will still leave 
					the ports configured as outputs, but the user can then change the port value.
						
	parameters:		timer		-Must be a valid TIMERn macro

	return:			void
*/
void stop_timer16_clock(uint8_t timer);


/*
	function:		enable_timer16_overflow_interrupt
	
	description:	Enables overflow interrupts for specified timer, which trigger when the timer wraps back around to 0.
					This function does not effect any other timer behavior.
						
	parameters:		timer		-which timer's interrupt should be enabled. Must be a valid TIMERn macro
	
	return:			void
*/
void enable_timer16_overflow_interrupt(uint8_t timer);



/*
	function:		disable_timer16_overflow_interrupt
	
	description:	Disables the overflow interrupt function of the specified timer. This function does not effect
					any other timer behavior.
						
	parameters:		timer		-which timer's interrupt to disable. Must be a valid TIMERn macro
	
	return:			void
*/
void disable_timer16_overflow_interrupt(uint8_t timer);


/*	
	function:		set_timer16_frequency
	
	description:	Sets the frequency and prescaler of the specified timer, stopping the timer for the duration of the
					function. This will effect any functions that rely on the specified timer for timing, so its 
					recommended that all timing related functions have their own dedicated timers. Note that not all 
					chosen frequencies are possible due to the timer's resolution, so the specified and actual frequency 
					can differ. See the ATmega1281 data sheet for specifics
	
	parameters:		timer			-the timer whose frequency is being changed. Must be a valid TIMERn macro
	
					frequency		-the desired frequency to set the timer to. 
					
					prescaler		-timer prescaler to use. Must be a valid TIMER16_PRESCALER macro
	
	return:			void
*/
void set_timer16_frequency(uint8_t timer, uint16_t frequency, uint8_t prescaler);

#endif