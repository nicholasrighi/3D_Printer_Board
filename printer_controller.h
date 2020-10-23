#ifndef PRINTER_CONTROLLER
#define PRINTER_CONTROLLER

/*
	These functions use the TIMER1_OVF_vect interrupt, which triggers when timer1 overflows. These functions 
	do not enable global interrupts on their own, however, so sei() must be called before move_printer_head is
	called to ensure that the timer will stop the printer head from moving at the correct time. Additionally, 
	leaving other interrupts on while the printer head is moving runs the risk of the ISR not finishing in time 
	and causing the printer head to overshoot. 
*/

#include <stdint.h>

#define COMMAND_LENGTH		x28		//maximum length of a single g-code command

/*
	function:		setup_printer 
	
	description:	sets up timers for pwm and timer interrupts. This function must be called before any calls to 
					move_pritner_head, otherwise move_printer_head will do nothing. This function associates
					the ports passed to this function as outputs and controls those pins, so those pins
					can't be used for other purposes while the printer is running. All ports passed to function
					must be unique and from different timers. This function also uses two pins to control the 
					direction of the motor, which can be found in printer_controller.c
	
	parameters:		x_out		-the output port that is connected to the x axis (must be one of A1,B1,C1,A3,B3,or C3)
					y_out		-the output port that is connected to the y axis (must be one of A1,B1,C1,A3,B3,or C3)	
					z_out		-the output port that is connected to the z axis (must be one of A1,B1,C1,A3,B3,or C3)
	
	return:			void
*/
void setup_printer(uint8_t x_port, uint8_t y_port, uint8_t z_port);

/*
	function:		move_printer_head
	
	description:	moves the printer head to point(x, y) such all axis finish moving at the same time. This point is 
					defined from the printer's  current position and not the origin. This function uses timer1 and
					timer3 to control the x and y axis, so timer1 and timer3 cant' be used for other functions
					while the print head is being controlled

	parameters:		x		-distance to move along the x axis (negative values move motor backwards)
					y		-distance to move along the y axis (negative values move motor backwards)
					
					note:	either x or y can be zero, but not both. Passing both x and y as zero could cause damage
							to the motors.
					
	return:			void
*/
void move_printer_xy(float x, float y);

/*
	function:		get_printer_status
	
	description:	returns TRUE if the printer has finished moving and FALSE otherwise. 
	
	parameters:		none
	
	return:			TRUE or FALSE
*/
uint8_t printer_finished_moving();


/*
	function:		takes a g-code command and moves the print head accordingly (this will also extrude filament if 
					necessary). 
	
	parameters:		buffer		-buffer containing the g-code command to be parsed. Buffer must be the size of
								 COMMAND_LENGTH
	
	return:			void
*/
void	move_via_gcode(char* buffer);

#endif