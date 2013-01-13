/********************************************************************************
*Copyright (C) 2013 	Moritz Nagel 	mnagel@htwg-konstanz.de					*
*                       Daniel Urbanietz   daniel@d-urbanietz.de                *
*********************************************************************************
*FILE: blue_os_utils.c                                                          *
*                                                                               *
*AUTHOR(S): M.Nagel                                                             *
*                                                                               *
*DESCRIPTION:                                                                   *
*                                                                               *
*                                                                               *
*VERSION: 1.0.                                                                  *
*                                                                               *			
*********************************************************************************
*This file is part of BlueOS.                                                   *
*                                                                               *
*BlueOS is free software: you can redistribute it and/or modify	                *
*it under the terms of the GNU Lesser General Public License as published by    *
*the Free Software Foundation, either version 3 of the License, or	            *
*(at your option) any later version.                                            *
*                                                                               *
*BlueOS is distributed in the hope that it will be useful,                      *
*but WITHOUT ANY WARRANTY; without even the implied warranty of	                *
*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                  *
*GNU Lesser General Public License for more details.                            *
*                                                                               *
*You should have received a copy of the GNU Lesser General Public License       *
*along with BlueOS.  If not, see <http://www.gnu.org/licenses/>.                *
********************************************************************************/

#include "blue_os_config.h"
#include "blue_os_utils.h"
#include "blue_os_internals.h"
#include "blue_os_hardware.h"

#include <avr/io.h>


#ifdef BLUE_OS_US_TIMER
uint16_t blueOsGetUs()
{
	// Prevent underflows, if the overflow interrupt has not yet been handled
	uint32_t temp_ticks = TIMER_COUNTER;

	if (temp_ticks > TIMER_COUNTER_INIT_VALUE)
		temp_ticks -= TIMER_COUNTER_INIT_VALUE;
	
	if (TIMER_INTERRUPT_FLAG_REGISTER & TIMER_INTERRUPT_FLAG)
	{
		// There was an timer overflow, but not yet handled.
		// To get the most accurate time, add the not yet handled ticks.
		// Consider, that the return value can be greater than 1000
		// in this case.

		#ifdef BLUE_OS_16_BIT_TIMER
		temp_ticks += 0xFFFF;
		#else
		temp_ticks += 0xFF;
		#endif //BLUE_OS_16_BIT_TIMER
	}
	return temp_ticks * TIMER_US_FACTOR;
}

uint32_t blueOsGetMs()
{
	return _milliseconds;
}

void blueOsSetMs(uint32_t time)
{
	_milliseconds = time;
}
#endif
