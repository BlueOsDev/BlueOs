/********************************************************************************
*Copyright (C) 2013 	Moritz Nagel 	mnagel@htwg-konstanz.de					*
*					Daniel Urbanietz 	daniel@d-urbanietz.de					*
*********************************************************************************
*FILE: blue_os_mem.h															*
*																				*
*AUTHOR(S): D.Urbanietz										  					*
*																				*
*DESCRIPTION:																	*
*This module provides a memory management for the BlueOS						*
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


#ifndef BLUE_OS_MEM_H
#define BLUE_OS_MEM_H

#include "blue_os_config.h"
#include <avr/io.h>

#ifdef BLUE_OS_MEMORY


#ifdef BLUE_OS_MEM_BUDDY
	#ifdef BLUE_OS_MEM_STANDARD
		#error Please choose only one memory management algorithm
	#endif	
#endif

#ifdef BLUE_OS_MEM_STANDARD
	#ifdef BLUE_OS_MEM_RotateFirstFit
		#ifdef BLUE_OS_MEM_FirstFit
			#error Please choose only one memory management algorithm
		#endif
		#ifdef BLUE_OS_MEM_BestFit
			#error Please choose only one memory management algorithm
		#endif
	#endif
	#ifdef BLUE_OS_MEM_FirstFit
		#ifdef BLUE_OS_MEM_RotateFirstFit 
			#error Please choose only one memory management algorithm
		#endif
		#ifdef BLUE_OS_MEM_BestFit
			#error Please choose only one memory management algorithm
		#endif
	#endif
	#ifdef BLUE_OS_MEM_BestFit
		#ifdef BLUE_OS_MEM_FirstFit
			#error Please choose only one memory management algorithm
		#endif
		#ifdef BLUE_OS_MEM_RotateFirstFit
			#error Please choose only one memory management algorithm
		#endif
	#endif
#endif




int8_t blue_os_mem_init();
int8_t blue_os_alloc(uint8_t** ptr, uint16_t bytes);
void blue_os_memChown(uint8_t* ptr, uint8_t taskId);

void blue_os_free(uint8_t* ptr);

//for internal uses 
void blue_os_freeKill(uint8_t taskId);



#endif 


#endif
