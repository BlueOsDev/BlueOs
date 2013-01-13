
/********************************************************************************
*Copyright (C) 2013 	Moritz Nagel 	mnagel@htwg-konstanz.de					*
*					Daniel Urbanietz 	daniel@d-urbanietz.de					*
*********************************************************************************
*FILE: blue_os_debug.h															*
*																				*
*AUTHOR(S): M. Nagel															*
*																				*
*DESCRIPTION:																	*
*This module provides some kernel debug functions. You can configure it in the	*
*\BlueOS\config\blue_os_config.h . 												*
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


#ifndef BLUE_OS_DEBUG_H
#define BLUE_OS_DEBUG_H

#include "blue_os.h"
#include "blue_os_config.h"

// Defines
#define BLUE_OS_PANIC_STACKPOINTER_OUTSIDE	1
#define BLUE_OS_PANIC_STACK_DAMAGED			2
#define BLUE_OS_PANIC_IDLE_TASK_RETURNED 	3

// Functions
#ifdef BLUE_OS_DEBUG
	
	
	#define blueOsAssert(e, r)	((e)?(void)0:blueOsKernelPanic(r))
#else
	#define blueOsAssert(e, r)
#endif //BLUE_OS_DEBUG

/**
 * This function is the kernel panic function.
 * It will halt the system for debugging purposes.
 *
 * @warning This function will not return.
 * @warning This function disables the interrupts
 *
 * @param errorValue ID of the error, that caused the kernel panic.
 */
void blueOsKernelPanic(volatile uint8_t errorValue);

#ifdef BLUE_OS_IMPLEMENT_PANIC_CALLBACK
/**
 * If BLUE_OS_IMPLEMENT_PANIC_CALLBACK is defined, this function
 * will be called in the panic procedure.
 *
 * @warning This function must be implemented by the user, if BLUE_OS_IMPLEMENT_PANIC_CALLBACK is defined!
 *
 * @param errorValue ID of the error, that caused the kernel panic.
 */
void blueOsKernelPanicHook(volatile uint8_t errorValue);
#endif //BLUE_OS_IMPLEMENT_PANIC_CALLBACK

#endif
