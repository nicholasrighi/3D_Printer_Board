#include <avr/interrupt.h>	
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

#include "printer_controller.h"
#include "Timers.h"
#include "global_macros.h"
#include "TMC_2208.h"
#include "io_macros.h"


#define CURRENT_TIMER_PRESCALER		TIMER16_PRESCALE_8
#define NOT_ASSIGNED				-1
#define STEP_FREQUENCY				1650				//any value below 1000 works well for motor frequency, any value higher
														//than 1000 will lead to motor stalling when it starts moving
																			
#define ABS(x)						((x)<0 ? -(x) : (x))
#define MIN_THRESHOLD				0.01				//minimum mm distance that the printer will move
												
typedef struct {
	uint32_t current_value;			//current number of steps taken in current movement
	uint32_t max_value;				//maximum number of steps needed for current movement
	uint32_t frequency;				//pwm frequency
	uint8_t port;					//16 bit timer port this axis is controlling
	volatile uint8_t done;			//flag for state of current movement
	uint8_t timer;					//16 bit timer being used by current movement
} axis_movement_info;

axis_movement_info x_axis;
axis_movement_info y_axis;
axis_movement_info z_axis;
		

ISR(TIMER1_OVF_vect) {
	
	if (x_axis.timer == TIMER1) {

		if (x_axis.current_value < x_axis.max_value) {
			x_axis.current_value++;
		} else {
			stop_timer16_clock(TIMER1);
			x_axis.done = TRUE;
		}
		
	} else {
		
		if (y_axis.current_value < y_axis.max_value) {
			y_axis.current_value++;
		} else {
			stop_timer16_clock(TIMER1);
			y_axis.done = TRUE;
		}
		
	}	
}

ISR(TIMER3_OVF_vect) {
	
	if (x_axis.timer == TIMER3) {
		
		if (x_axis.current_value < x_axis.max_value) {
			x_axis.current_value++;
		} else {
			stop_timer16_clock(TIMER3);
			x_axis.done = TRUE;
		}
		
	} else {
		
		if (y_axis.current_value < y_axis.max_value) {
			y_axis.current_value++;
		} else {
			stop_timer16_clock(TIMER3);
			y_axis.done = TRUE;
		}
		
	}
	
}


void setup_printer(uint8_t x_port, uint8_t y_port, uint8_t z_port) {

	
	x_axis.port = x_port;
	y_axis.port = y_port;
	z_axis.port = z_port;
	
	if (x_port == A1 || x_port == B1 || x_port == C1) {
		x_axis.timer = TIMER1;
		y_axis.timer = TIMER3;
	} else if (x_port == A3 || x_port == B3 || x_port == C3) {
		x_axis.timer =  TIMER3;
		y_axis.timer = TIMER1;
	}
	
	SET_PIN_OUTPUT(F, 0);
	SET_PIN_OUTPUT(F, 1);
	
	//ATmega1281 only has two 16 bit timers, and the x and y axis each need one of them
	enable_timer16_overflow_interrupt(TIMER1);
	enable_timer16_overflow_interrupt(TIMER3);
	
	//drive enable pin low to connect motor to controller. This prevents motor from moving
	//due to noise until the drivers have been set up. 
	SET_PIN_OUTPUT(A, 0);
	SET_PIN_LOW(A, 0);	
	
}

/*
	Moving x millimeters requires x/MM_PER_FULL_ROTATION full rotations, and each full step requires
	FULL_STEPS_PER_ROTATION steps. Each full step requires MAX_MICROSTEPS_PER_FULLSTEP/(2 ^ CURRENT_MICROSTEP_VALUE) 
	pwm pulses on the TMC2208's STEP pin. This means to move x millimeter we need 
	(x * FULL_STEPS_PER_ROTATION * MAX_MICROSTEPS_PER_FULLSTEP) / (MM_PER_FULL_ROTATION * (2 ^ CURRENT_MICROSTEP_VALUE))
	pulses on the STEP pin of the TMC2208
*/
void move_printer_xy(float x, float y) {
	
	//control motor direction depending on sign of x and y
	if (x < 0) {
		SET_PIN_LOW(F, 0);
	} else {
		SET_PIN_HIGH(F, 0);
	} 
	
	if (y < 0) {
		SET_PIN_LOW(F, 1);
	} else {
		SET_PIN_HIGH(F, 1);
	}
	
	float abs_x = ABS(x);
	float abs_y = ABS(y);
	
	x_axis.done = FALSE;
	y_axis.done = FALSE;
	
	x_axis.current_value = 0;
	y_axis.current_value = 0;
	
	x_axis.max_value = (abs_x* FULL_STEPS_PER_ROTATION * MAX_MICROSTEP_PER_FULL_STEP) / (MM_PER_FULL_ROTATION * (1 << CURRENT_MICROSTEP_VALUE));
	y_axis.max_value = (abs_y* FULL_STEPS_PER_ROTATION * MAX_MICROSTEP_PER_FULL_STEP) / (MM_PER_FULL_ROTATION * (1 << CURRENT_MICROSTEP_VALUE));
	
	if (abs_x > abs_y) {
		x_axis.frequency = STEP_FREQUENCY;
		y_axis.frequency = (abs_y* STEP_FREQUENCY)/abs_x;
	} else {
		y_axis.frequency = STEP_FREQUENCY;
		x_axis.frequency = (abs_x* STEP_FREQUENCY)/abs_y;
	} 
	
	
	//only move axis whose displacement value is non zero
	if (abs_x >= MIN_THRESHOLD) {
		init_pwm_timer16(x_axis.port, x_axis.frequency, CURRENT_TIMER_PRESCALER, 0.5);
	}
	
	if (abs_y >= MIN_THRESHOLD) {
		init_pwm_timer16(y_axis.port,  y_axis.frequency, CURRENT_TIMER_PRESCALER, 0.5);
	}
	
	
	
}

uint8_t printer_finished_moving() {
	if (x_axis.done == TRUE && y_axis.done == TRUE) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void move_via_gcode(char* buffer) {
	
	char command_type = buffer[0];
	char command_number = buffer[1];
	
	char* x_start;
	char* y_start;
	char* z_start;
	
	float x_val;
	float y_val;
	float z_val;

	//parse g-code command
	switch(command_type)
	{
		case 'G':
		switch(command_number)
		{
			case '0':
			{

				x_start = strchr(buffer, 'X');
				y_start = strchr(buffer, 'Y');
				z_start = strchr(buffer, 'Z');

				//note that current implementation ignore decimals,
				//so all decimals are truncated to integers
				if (x_start != NULL) {
					x_val = atof(x_start + 1);
				} else {
					x_val = 0;
				}
				
				if (y_start != NULL) {
					y_val = atof(y_start + 1);
				} else {
					y_val = 0;
				}

				if (z_start != NULL) {
					z_val = atof(z_start + 1);
				}
				
				if (ABS(x_val) > MIN_THRESHOLD || ABS(y_val) > MIN_THRESHOLD) {
					move_printer_xy(x_val, y_val);
				} 
				
				break;
			}		
			
		}
	}
}