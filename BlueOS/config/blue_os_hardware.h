
/********************************************************************************
*Copyright (C) 2013 	Moritz Nagel 	mnagel@htwg-konstanz.de					*
*					Daniel Urbanietz 	daniel@d-urbanietz.de					*
*********************************************************************************
*FILE: blue_os_hardware.h														*
*																				*
*AUTHOR(S): M. Nagel, D. Urbanietz												*
*																				*
*DESCRIPTION:																	*
*This files includes the hardware specific include files						*
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


#ifndef BLUE_OS_HARDWARE_H
#define BLUE_OS_HARDWARE_H

#include <avr/io.h>

#if defined (__AVR_ATmega8__)

	#include "blue_hw_atmeg8.h"

#elif defined (__AVR_ATmega32__)

	#include "blue_hw_atmeg32.h"

#elif defined (__AVR_ATmega128__)

	#include "blue_hw_atmeg128.h"

#elif defined (__AVR_ATmega644P__)

	#include "blue_hw_atmeg644P.h"

#elif defined (__AVR_ATxmega128A1__)

	#include "blue_hw_atxmega128a1.h"

#else
	#error "unsupported device"
#endif


#endif
