/********************************************************************************
*Copyright (C) 2013 	Moritz Nagel 	mnagel@htwg-konstanz.de					*
*					Daniel Urbanietz 	daniel@d-urbanietz.de					*
*********************************************************************************
*FILE: blue_os_mem.c															*
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


#include "blue_os_mem.h"
#include "blue_os.h"
#include "blue_os_internals.h"
#include <avr/io.h>

#ifdef BLUE_OS_MEMORY

#ifdef BLUE_OS_MEM_BUDDY
	#if BLUE_OS_TASK_COUNT > 8
		typedef struct blue_mem_block
		{
			uint8_t task_id : 8;	
			uint8_t size : 4;
			uint8_t free : 1;
		} blue_mem_block;
	#else 
		typedef struct blue_mem_block
		{
			uint8_t task_id : 3;	
			uint8_t size : 4;
			uint8_t free : 1;
		} blue_mem_block;
	#endif
#endif //BLUE_OS_MEM_BUDDY	

#ifdef BLUE_OS_MEM_STANDARD
	typedef struct blue_mem_block
	{
		uint8_t task_id;	
		uint8_t size ;
		uint8_t free;
	} blue_mem_block;
	#ifdef BLUE_OS_MEM_RotateFirstFit
		static uint8_t lastIndex = 0;
	#endif //BLUE_OS_MEM_RotateFirstFit
#endif //BLUE_OS_MEM_STANDARD


void blue_mem_merge();
uint16_t blue_mem_find(uint8_t* ptr, uint8_t* index);

#define BUE_OS_MEM_BLOCK_COUNT BLUE_OS_MEM_SIZE / BLUE_OS_MEMBLOCK_SIZE

static uint8_t blue_mem[BLUE_OS_MEM_SIZE];
static blue_mem_block blue_mem_table[BUE_OS_MEM_BLOCK_COUNT];

#ifdef BLUE_OS_MEM_BUDDY
	static uint8_t max_power;
	static uint8_t min_power;
#endif //BLUE_OS_MEM_BUDDY


int8_t blue_os_mem_init()
{
	uint16_t i=0;

#ifdef BLUE_OS_MEM_BUDDY
	int8_t j = 0;
	max_power = 16;
	min_power = 16;
	uint16_t tmp = BLUE_OS_MEM_SIZE - 1;

	for(j=15; j>=0;j--)
	{
		if(!(tmp & (1 << j)))
			max_power--;
		else
			break;		
	}

	tmp = BLUE_OS_MEMBLOCK_SIZE - 1 ;
	for(j=15; j>=0;j--)
	{
		if(!(tmp & (1 << j)))
			min_power--;
		else
			break;		
	}
#endif //BLUE_OS_MEM_BUDDY

	for(i=0; i<BUE_OS_MEM_BLOCK_COUNT; i++)
	{
		blue_mem_table[i].free = 1;
		#ifdef BLUE_OS_MEM_BUDDY
			blue_mem_table[i].size = max_power; //at the beginning all blocks are one area
		#endif //BLUE_OS_MEM_BUDDY

		#ifdef BLUE_OS_MEM_STANDARD
			blue_mem_table[i].size = BUE_OS_MEM_BLOCK_COUNT-1;
		#endif //BLUE_OS_MEM_STANDARD

	}
	
	return 0;
}

int8_t blue_os_alloc(uint8_t** ptr, uint16_t bytes)
{
	uint16_t i;
	uint16_t step = 1;
	uint8_t index =0;
	uint8_t crit;
#ifdef BLUE_OS_MEM_BUDDY
	uint8_t need_power = 16;
	uint8_t best_power = max_power + 1;
	int8_t k = 0;
#endif //BLUE_OS_MEM_BUDDY

#ifdef BLUE_OS_MEM_STANDARD
	uint16_t blocks = 0;
	#ifdef BLUE_OS_MEM_BestFit
		uint16_t bestSize = BUE_OS_MEM_BLOCK_COUNT;
	#endif //BLUE_OS_MEM_BestFit
#endif //BLUE_OS_MEM_STANDARD

#ifdef BLUE_OS_MEM_BUDDY
	
	for(k=15; k>=0;k--)
	{
		if(!((bytes-1) & (1 << k)))
			need_power--;
		else
			break;		
	}
	if(need_power < min_power)
		need_power = min_power;
#endif //BLUE_OS_MEM_BUDDY

#ifdef BLUE_OS_MEM_STANDARD
	blocks = bytes / BLUE_OS_MEMBLOCK_SIZE;
	if(bytes % BLUE_OS_MEMBLOCK_SIZE)
		blocks++;
	blocks--;
#endif //BLUE_OS_MEM_STANDARD	

crit = blueOsEnterCriticalSection();


//FIND SUITABLE BLOCK/////////////////////////////////////////////////
#ifdef BLUE_OS_MEM_BUDDY
	//search best fitting block
	i=0;
	while(i < BUE_OS_MEM_BLOCK_COUNT)
	{		
		step = (1 << blue_mem_table[i].size) / BLUE_OS_MEMBLOCK_SIZE;
		
		if(blue_mem_table[i].free && (blue_mem_table[i].size >= need_power) && (blue_mem_table[i].size < best_power))
		{		
			best_power = blue_mem_table[i].size;
			index = i;	
		}
		i = i + step;
		if(step == 0)
			break;
	}

	if(best_power > max_power)
	{
		blueOsLeaveCriticalSection(crit);
		return -1;
	}

	//Split block
	step = ((1 << blue_mem_table[index].size) / BLUE_OS_MEMBLOCK_SIZE) -1;
	while((blue_mem_table[index].size - 1) >= need_power)
	{
		for(i=0;i<=step;i++)
		{
			blue_mem_table[index+i].size--;
		}
		step = step / 2;	
	}
#endif //BLUE_OS_MEM_BUDDY

#ifdef BLUE_OS_MEM_STANDARD
	#ifdef BLUE_OS_MEM_RotateFirstFit
		i = lastIndex;
		while(1)
		{
			if(blue_mem_table[i].size >= blocks)
			{
				index = i;
				lastIndex = i;
				break;
			} 
			i += blue_mem_table[i].size + 1;
			if(i >= BUE_OS_MEM_BLOCK_COUNT)
			{
				i = 0;
			}
			if(i == lastIndex)
			{
				blueOsLeaveCriticalSection(crit);
				return -1;
			}
		}
	#endif //BLUE_OS_MEM_RotateFirstFit

	#ifdef BLUE_OS_MEM_FirstFit
		i = 0;
		while(i < BUE_OS_MEM_BLOCK_COUNT)
		{
			if(blue_mem_table[i].size >= blocks)
			{
				index = i;
				break;
			} 
			i += blue_mem_table[i].size + 1;
		}
		if(i >= BUE_OS_MEM_BLOCK_COUNT)
		{
			blueOsLeaveCriticalSection(crit);
			return -1;
		}
	#endif //BLUE_OS_MEM_FirstFit

	#ifdef BLUE_OS_MEM_BestFit
		i = 0;
		while(i < BUE_OS_MEM_BLOCK_COUNT)
		{
			if(blue_mem_table[i].size >= blocks && bestSize > blue_mem_table[i].size )
			{
				bestSize = blue_mem_table[i].size; 
				index = i;
			} 
			i += blue_mem_table[i].size + 1;
		}
		if(bestSize >= BUE_OS_MEM_BLOCK_COUNT)
		{
			blueOsLeaveCriticalSection(crit);
			return -1;
		}
	#endif //BLUE_OS_MEM_BestFit
#endif //BLUE_OS_MEM_STANDARD
//////////////////////////////////////////////////////////////////////	

#ifdef BLUE_OS_MEM_BUDDY			
	step = (1 << blue_mem_table[index].size) / BLUE_OS_MEMBLOCK_SIZE;
	//mark block as used
	for(i=0;i<step;i++)
	{
		blue_mem_table[index+i].task_id = _currentTaskId;
		blue_mem_table[index+i].free = 0;
	}
#endif //BLUE_OS_MEM_BUDDY

#ifdef BLUE_OS_MEM_STANDARD
	step = blue_mem_table[index].size + index;
	for(i=index; i<=step;i++)
	{
		if(i<=(blocks + index))
		{
			blue_mem_table[i].task_id = _currentTaskId;
			blue_mem_table[i].free = 0;
			blue_mem_table[i].size = blocks;
		}
		else
		{
			blue_mem_table[i].size -= (blocks+1);
		}
	} 
#endif //BLUE_OS_MEM_STANDARD

	*ptr = blue_mem + (index*BLUE_OS_MEMBLOCK_SIZE);
	blueOsLeaveCriticalSection(crit);
	return 0;
}



void blue_os_free(uint8_t* ptr)
{
	uint8_t i;
	uint16_t j;
	volatile uint16_t step = 0;
	uint8_t crit;

	crit = blueOsEnterCriticalSection();
	//free
	step = blue_mem_find(ptr, &i);
	for(j=0; j<step;j++)
	{
		blue_mem_table[i+j].free = 1;
	}
	if(step)
	{
		blue_mem_merge();
	}

	blueOsLeaveCriticalSection(crit);
}

#ifndef BLUE_OS_TASKS_NEVER_DEAD
void blue_os_freeKill(uint8_t taskId)
{
	uint8_t i;
	for(i=0;i<BUE_OS_MEM_BLOCK_COUNT;i++)
	{
		if(blue_mem_table[i].task_id == taskId)
		{
			blue_mem_table[i].free = 1;
		}
	}
	blue_mem_merge();

}
#endif

void blue_os_memChown(uint8_t* ptr, uint8_t taskId)
{
	uint8_t j,i,step;
	uint8_t crit = blueOsEnterCriticalSection();

	step = blue_mem_find(ptr, &i);
	for(j=0; j<step;j++)
	{
		blue_mem_table[i+j].task_id = taskId;
	}

	blueOsLeaveCriticalSection(crit);	

}

uint16_t blue_mem_find(uint8_t* ptr, uint8_t* index)
{
	uint8_t* ref_ptr = blue_mem;
	uint16_t i = 0;
	uint16_t step;
	while(i < BUE_OS_MEM_BLOCK_COUNT)
	{		
#ifdef BLUE_OS_MEM_BUDDY
		step = (1 << blue_mem_table[i].size) / BLUE_OS_MEMBLOCK_SIZE;
#endif //BLUE_OS_MEM_BUDDY

#ifdef BLUE_OS_MEM_STANDARD
		step = blue_mem_table[i].size + 1;
#endif //BLUE_OS_MEM_STANDARD

		if(ref_ptr == ptr)
		{			
			*index = i;
			return step;
		}
		ref_ptr += step * BLUE_OS_MEMBLOCK_SIZE;
			 
		i = i + step;
	}
	return 0;

}

void blue_mem_merge()
{
	uint16_t j,i;
	#ifdef BLUE_OS_MEM_BUDDY
	uint8_t left = 1;
	#endif //BLUE_OS_MEM_BUDDY
	uint16_t step = 0;
	//merge 
	i = 0;
	while(i < BUE_OS_MEM_BLOCK_COUNT)
	{		
#ifdef BLUE_OS_MEM_BUDDY
		step = (1 << blue_mem_table[i].size) / BLUE_OS_MEMBLOCK_SIZE;
		if(left && blue_mem_table[i].size == blue_mem_table[i+step].size && blue_mem_table[i+step].free && blue_mem_table[i].free)
		{
			for(j=0; j< 2*step;j++)
				blue_mem_table[i+j].size++;
			
			left = 1;
			i = 0;
			continue;			
		}
		else if(!left && blue_mem_table[i].size == blue_mem_table[i-1].size && blue_mem_table[i-1].free && blue_mem_table[i].free)
		{
			for(j=0; j< 2*step;j++)
				blue_mem_table[i-step+j].size++;
			
			left = 1;
			i = 0;
			continue;			
		}
			
				
		if( (left && blue_mem_table[i].size <= blue_mem_table[i+step].size) ||
			(!left && blue_mem_table[i].size >= blue_mem_table[i+step].size)   )
		{
			left = !left;
		}
#endif //BLUE_OS_MEM_BUDDY

#ifdef BLUE_OS_MEM_STANDARD
		step = blue_mem_table[i].size + 1;
		if(blue_mem_table[i].free && (i+step)<BUE_OS_MEM_BLOCK_COUNT && blue_mem_table[i+step].free)
		{
			j = blue_mem_table[i+step].size + 1;
			step = step + j; 
			#ifdef BLUE_OS_MEM_RotateFirstFit
				if(lastIndex >= i && lastIndex < (i+step))
					lastIndex = i;
			#endif //BLUE_OS_MEM_RotateFirstFit
			for(j=0;j<step;j++)
			{
				blue_mem_table[i+j].size = step - 1;
			}
			i = 0;
			continue;
		}
#endif //BLUE_OS_MEM_STANDARD
			 
		i = i + step;
	}
}

#endif 
