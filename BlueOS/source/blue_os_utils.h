/********************************************************************************
*Copyright (C) 2013 	Moritz Nagel 	mnagel@htwg-konstanz.de					*
*					Daniel Urbanietz 	daniel@d-urbanietz.de					*
*********************************************************************************
*FILE: blue_os_utils.h															*
*																				*
*AUTHOR(S): M.Nagel 															*
*																				*
*DESCRIPTION:																	*
*								 												*
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

#ifndef BLUE_OS_UTILS_H
#define BLUE_OS_UTILS_H

#include "blue_os_config.h"

#include <avr/io.h>

#ifdef BLUE_OS_US_TIMER
/**
 * This function returns the microseconds since the last millisecond.
 *
 * @return Microseconds since the last millisecond.
 */
uint16_t blueOsGetUs();

/**
 * This function returns the overall milliseconds since power-on.
 *
 * @return Overall milliseconds since power-on.
 */
uint32_t blueOsGetMs();

/**
 * This function sets the system time. Can be used, to synchronize the
 * controller with other controllers.
 *
 * @param time Overall milliseconds since power-on.
 */
void blueOsSetMs(uint32_t time);
#endif

#endif
