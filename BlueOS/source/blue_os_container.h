
/********************************************************************************
*Copyright (C) 2013 	Moritz Nagel 	mnagel@htwg-konstanz.de					*
*					Daniel Urbanietz 	daniel@d-urbanietz.de					*
*********************************************************************************
*FILE: blue_os_container.h														*
*																				*
*AUTHOR(S): M. Nagel, D. Urbanietz												*
*																				*
*DESCRIPTION:																	*
*This module provides a queue-based container structure. 						*
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


#ifndef BLUE_OS_CONTAINER_H
#define BLUE_OS_CONTAINER_H

#include "blue_os_config.h"
#include "blue_os_synchronization.h"
#include "blue_os_callBack.h"

#include <avr/io.h>

// Definitions
#define BLUE_OS_QUEUE_POLLING 0x01

// Typedefs

typedef volatile struct BlueOsQueue
{
	volatile uint8_t* _queue;
	volatile BLUE_OS_QUEUE_DATA_TYPE _queuesize;
	volatile BLUE_OS_QUEUE_DATA_TYPE _queueBegin;
	volatile BLUE_OS_QUEUE_DATA_TYPE _queueEnd;
	volatile BLUE_OS_QUEUE_DATA_TYPE _elementcount;

	volatile BlueOsCallBack* _callBack;

	#ifdef BLUE_OS_USE_SIGNAL
	BlueOsSignal* _emptySignal;
	BlueOsSignal* _fullSignal;
	#endif //BLUE_OS_USE_SIGNAL
} BlueOsQueue;


// Functions

/**
 * This function initializes an 8-bit queue object.
 *
 * @param queueObject Pointer to the queue object.
 * @param queue Pointer to the queue (array of uint8_t).
 * @param queueSize Size of the array, passed in "queue".
 */
void blueOsQueueInit(volatile BlueOsQueue* queueObject, volatile uint8_t* queue, BLUE_OS_QUEUE_DATA_TYPE queueSize);


void blueOsQueueAddCallBack(volatile BlueOsQueue* queueObject, BlueOsCallBack* callBack );
void blueOsQueueRemoveCallBack(volatile BlueOsQueue* queueObject, BlueOsCallBack* callBack);

void blueOsQueueFlush(volatile BlueOsQueue* queueObject);

#ifdef BLUE_OS_USE_SIGNAL
void blueOsQueueInitBlocking(volatile BlueOsQueue* queueObject, volatile uint8_t* queue, BLUE_OS_QUEUE_DATA_TYPE queueSize, struct BlueOsSignal* emptySignal, struct BlueOsSignal* fullSignal);
#endif //BLUE_OS_USE_SIGNAL

/**
 * This function enqueues an 8-bit-value into a queue.
 *
 * @param queueObject Pointer to the queue object, where the value is enqueued.
 * @param value 8-bit-value, that is enqueued.
 *
 * @return	 0: Value successfully enqueued.
 *			-1: Value could not be enqueued.
 */
int8_t blueOsQueueEnqueue(volatile BlueOsQueue* queueObject, uint8_t value);

/**
 * This function dequeues an 8-bit-value from a queue.
 *
 * @param queueObject Pointer to the queue object, where the value is dequeued.
 * @param retvalue Poiner to an 8-bit variable, where the dequeued value can be stored.
 *
 * @return	 0: Value successfully dequeued.
 *			-1: Value could not be dequeued.
 */
int8_t blueOsQueueDequeue(volatile BlueOsQueue* queueObject, uint8_t* retvalue);

BLUE_OS_QUEUE_DATA_TYPE blueOsQueueEnqueueBlock(volatile BlueOsQueue* queueObject,
								const uint8_t* data, BLUE_OS_QUEUE_DATA_TYPE length,
								uint8_t options);

BLUE_OS_QUEUE_DATA_TYPE blueOsQueueDequeueBlock(volatile BlueOsQueue* queueObject,
								uint8_t* data, BLUE_OS_QUEUE_DATA_TYPE length,
								uint8_t options);
#endif
