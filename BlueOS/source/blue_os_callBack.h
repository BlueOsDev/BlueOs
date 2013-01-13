
/********************************************************************************
*Copyright (C) 2013 	Moritz Nagel 	mnagel@htwg-konstanz.de					*
*					Daniel Urbanietz 	daniel@d-urbanietz.de					*
*********************************************************************************
*FILE: blue_os_callBack.h														*
*																				*
*AUTHOR(S): D. Urbanietz														*
*																				*
*DESCRIPTION:																	*
*This module provides the general callBack structure used by the BlueOS. 		*
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
#ifndef BLUE_OS_CALLBACK_H
#define BLUE_OS_CALLBACK_H

#include <avr/io.h>

typedef void (*CallBackFunc)(uint8_t* source, uint8_t value);

typedef volatile struct BlueOsCallBack
{
	volatile struct BlueOsCallBack* next;
	CallBackFunc func;	
}BlueOsCallBack;

#endif
