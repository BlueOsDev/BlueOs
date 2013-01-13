
/********************************************************************************
*Copyright (C) 2013 	Moritz Nagel 	mnagel@htwg-konstanz.de					*
*					Daniel Urbanietz 	daniel@d-urbanietz.de					*
*********************************************************************************
*FILE: blue_os.h																*
*																				*
*AUTHOR(S): M. Nagel, D. Urbanietz												*
*																				*
*DESCRIPTION:																	*
*This module contains the scheduler and dispatcher of the BlueOS				*
*																				*
*VERSION: 1.0.																	*
*																				*			
*********************************************************************************
*This file is part of BlueOS.													*
*																				*
*BlueOS is free software: you can redistribute it and/or modify					*
*it under the terms of the GNU Lesser General Public License as published by	*
*the Free Software Foundation, either version 3 of the License, or				*
*(at your option) any later version.											*
*																				*
*BlueOS is distributed in the hope that it will be useful,						*
*but WITHOUT ANY WARRANTY; without even the implied warranty of					*
*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the					*
*GNU Lesser General Public License for more details.							*
*																				*
*You should have received a copy of the GNU Lesser General Public License		*
*along with BlueOS.  If not, see <http://www.gnu.org/licenses/>.				*
********************************************************************************/


#ifndef BLUE_OS_H
#define BLUE_OS_H

#include "blue_os_container.h"

#include "blue_os_hardware.h"
#include "blue_os_synchronization.h"
#include "blue_os_task.h"

#include "blue_os_shell.h"
#include "blue_os_shell_vt100.h"

#include "blue_os_utils.h"
#include "blue_os_mem.h"


#include <avr/io.h>


// Functions

/**
 * Starts the scheduler.
 * The function, which calls this function will become the idle task!
 */
void blueOsInit();

#ifdef BLUE_OS_SCHEDULER_OWN_IMPLEMENTATION
/**
 * If BLUE_OS_SCHEDULER_OWN_IMPLEMENTATION is defined,
 * this function must be implemented by the user! It selects the
 * next task, that will run, after a context switch.
 *
 * @warn You MUST return the idle task ID (which is 0), if
 * no other task can be scheduled! Returning an invalid task ID
 * will bring the OS into an undefined state.
 *
 * @return Task ID, of the next task.
 */
uint8_t blueOsScheduleOwnImplementation() __attribute__ ((always_inline));
#endif //BLUE_OS_SCHEDULER_OWN_IMPLEMENTATION

#endif
