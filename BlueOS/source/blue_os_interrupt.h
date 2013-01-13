#ifndef BLUE_OS_INTERRUPT_H
#define BLUE_OS_INTERRUPT_H

#include <avr/interrupt.h>

#define BLUE_OS_ISR(vector, user_implementation)				\
	void vector (void) __attribute__ ((signal,__INTR_ATTRS)); 	\
    void vector (void) {										\
		char sreg_backup = SREG;								\
		user_implementation();									\
		SREG = sreg_backup;										\
	}

#endif
