/********************************************************************************
*Copyright (C) 2013 	Moritz Nagel 	mnagel@htwg-konstanz.de					*
*					Daniel Urbanietz 	daniel@d-urbanietz.de					*
*********************************************************************************
*FILE: blue_os_container.c														*
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

#include "blue_os_container.h"
#include "blue_os.h"

// <Queue>
void blueOsQueueInit(volatile BlueOsQueue* queueObject, volatile uint8_t* queue, BLUE_OS_QUEUE_DATA_TYPE queueSize)
{
	uint8_t irstate = blueOsEnterCriticalSection();
	
	queueObject->_queue = queue;
	queueObject->_queuesize = (BLUE_OS_QUEUE_DATA_TYPE) queueSize;
	queueObject->_queueBegin = (BLUE_OS_QUEUE_DATA_TYPE) 0;
	queueObject->_queueEnd = (BLUE_OS_QUEUE_DATA_TYPE) 0;
	queueObject->_elementcount = (BLUE_OS_QUEUE_DATA_TYPE) 0;

	queueObject->_callBack = 0;

	#ifdef BLUE_OS_USE_SIGNAL
	queueObject->_emptySignal = 0;
	queueObject->_fullSignal = 0;
	#endif //BLUE_OS_USE_SIGNAL

	blueOsLeaveCriticalSection(irstate);
}


#ifdef BLUE_OS_USE_SIGNAL
void blueOsQueueInitBlocking(volatile BlueOsQueue* queueObject,
								volatile uint8_t* queue,
								BLUE_OS_QUEUE_DATA_TYPE queueSize,
								struct BlueOsSignal* emptySignal,
								struct BlueOsSignal* fullSignal)
{
	blueOsQueueInit(queueObject, queue, queueSize);

	queueObject->_emptySignal = emptySignal;
	queueObject->_fullSignal = fullSignal;
}


#endif //BLUE_OS_USE_SIGNAL

void blueOsQueueAddCallBack(volatile BlueOsQueue* queueObject, BlueOsCallBack* callBack )
{
	uint8_t irstate = blueOsEnterCriticalSection();
	callBack->next = queueObject->_callBack;
	queueObject->_callBack = callBack;
	blueOsLeaveCriticalSection(irstate);
}

void blueOsQueueRemoveCallBack(volatile BlueOsQueue* queueObject, BlueOsCallBack* callBack )
{
	uint8_t irstate = blueOsEnterCriticalSection();
	BlueOsCallBack* tmp = queueObject->_callBack;
	if(tmp == callBack)
		 queueObject->_callBack = callBack->next;
	while(tmp)
	{
		if(tmp->next == callBack)
		{ 
			queueObject->_callBack = callBack->next;
			break;
		}
		tmp = tmp->next;		
	}
	blueOsLeaveCriticalSection(irstate);
}

void blueOsQueueFlush(volatile BlueOsQueue* queueObject)
{
	uint8_t irstate = blueOsEnterCriticalSection();
	queueObject->_queueBegin = (BLUE_OS_QUEUE_DATA_TYPE) 0;
	queueObject->_queueEnd = (BLUE_OS_QUEUE_DATA_TYPE) 0;
	queueObject->_elementcount = (BLUE_OS_QUEUE_DATA_TYPE) 0;
	#ifdef BLUE_OS_USE_SIGNAL
	if (queueObject->_emptySignal != 0)
	{
		blueOsSignalBroadcast(queueObject->_emptySignal);
	}
	#endif //BLUE_OS_USE_SIGNAL
	blueOsLeaveCriticalSection(irstate);
}

int8_t blueOsQueueEnqueue(volatile BlueOsQueue* queueObject, uint8_t value)
{
	// Registers for optimization
	BLUE_OS_QUEUE_DATA_TYPE queueEnd;
	BLUE_OS_QUEUE_DATA_TYPE queuesize;
	BLUE_OS_QUEUE_DATA_TYPE elementcount;

	BlueOsCallBack* tmp = queueObject->_callBack;

	#ifdef BLUE_OS_USE_SIGNAL
	while (1)
	{
	#endif //BLUE_OS_USE_SIGNAL

		uint8_t irstate = blueOsEnterCriticalSection();

		// Optimization: get local copy
		queuesize = queueObject->_queuesize;

		// Optimization: get local copy
		elementcount = queueObject->_elementcount;

		if (elementcount < queuesize)
		{
			// Optimization: get local copy
			queueEnd = queueObject->_queueEnd;

			queueObject->_queue[queueEnd] = value;
			queueEnd++;
		
			if (queueEnd >= queuesize)
				queueEnd = (BLUE_OS_QUEUE_DATA_TYPE) 0;

			// Optimization: store local copy
			queueObject->_queueEnd = queueEnd;
		
			elementcount++;

			// Optimization: store local copy
			queueObject->_elementcount = elementcount;

			blueOsLeaveCriticalSection(irstate);

			#ifdef BLUE_OS_USE_SIGNAL
			if (queueObject->_fullSignal != 0)
			{
				blueOsSignalBroadcast(queueObject->_fullSignal);
			}
			#endif //BLUE_OS_USE_SIGNAL

			while(tmp)
			{
				tmp->func((uint8_t*)queueObject,value);
				tmp = tmp->next;
			}			

			return (int8_t) 0;
		}
		blueOsLeaveCriticalSection(irstate);

	#ifdef BLUE_OS_USE_SIGNAL
		if (queueObject->_emptySignal != 0)
		{
			blueOsWait(queueObject->_emptySignal);
		}
		else
		{
			break;
		}

	}
	#endif //BLUE_OS_USE_SIGNAL

	return (int8_t) -1;
}

int8_t blueOsQueueDequeue(volatile BlueOsQueue* queueObject, uint8_t* retvalue)
{
	// Registers for optimization
	BLUE_OS_QUEUE_DATA_TYPE queueBegin;
	BLUE_OS_QUEUE_DATA_TYPE elementcount;

	#ifdef BLUE_OS_USE_SIGNAL
	while (1)
	{
	#endif //BLUE_OS_USE_SIGNAL	

		uint8_t irstate = blueOsEnterCriticalSection();

		// Optimization: get local copy
		elementcount = queueObject->_elementcount;

		if (elementcount > 0)
		{
			// Optimization: get local copy
			queueBegin = queueObject->_queueBegin;

			*retvalue = queueObject->_queue[queueBegin];
		
			queueBegin++;
			if (queueBegin >= queueObject->_queuesize)
				queueBegin = (BLUE_OS_QUEUE_DATA_TYPE) 0;
		
			// Optimization: store local copy
			queueObject->_queueBegin = queueBegin;
		
			elementcount--;

			// Optimization: store local copy
			queueObject->_elementcount = elementcount;

			blueOsLeaveCriticalSection(irstate);

			#ifdef BLUE_OS_USE_SIGNAL
			if (queueObject->_emptySignal != 0)
			{
				blueOsSignalBroadcast(queueObject->_emptySignal);
			}
			#endif //BLUE_OS_USE_SIGNAL


			return (int8_t) 0;
		}

		blueOsLeaveCriticalSection(irstate);

	#ifdef BLUE_OS_USE_SIGNAL
		if (queueObject->_fullSignal != 0)
		{
			blueOsWait(queueObject->_fullSignal);
		}
		else
		{
			break;
		}

	}
	#endif //BLUE_OS_USE_SIGNAL

	return (int8_t) -1;
}

BLUE_OS_QUEUE_DATA_TYPE blueOsQueueEnqueueBlock(volatile BlueOsQueue* queueObject,
								const uint8_t* data, BLUE_OS_QUEUE_DATA_TYPE length,
								uint8_t options)
{
	BLUE_OS_QUEUE_DATA_TYPE queueEnd;
	BLUE_OS_QUEUE_DATA_TYPE queuesize;
	BLUE_OS_QUEUE_DATA_TYPE elementcount;
	BLUE_OS_QUEUE_DATA_TYPE copyedCount = 0;


	#ifdef BLUE_OS_USE_SIGNAL
	while (1)
	{
	#endif //BLUE_OS_USE_SIGNAL

		uint8_t irstate = blueOsEnterCriticalSection();

		// Optimization: get local copy
		queuesize = queueObject->_queuesize;

		// Optimization: get local copy
		elementcount = queueObject->_elementcount;

		// Optimization: get local copy
		queueEnd = queueObject->_queueEnd;


		for ( ; (copyedCount < length) && (elementcount < queuesize); copyedCount++)
		{
			queueObject->_queue[queueEnd] = data[copyedCount];

			queueEnd++;

			queueEnd = queueEnd % queuesize;
			
			elementcount++;
		}
		
		// Optimization: store local copy
		queueObject->_queueEnd = queueEnd;

		// Optimization: store local copy
		queueObject->_elementcount = elementcount;

		blueOsLeaveCriticalSection(irstate);

		#ifdef BLUE_OS_USE_SIGNAL
		if (queueObject->_fullSignal != 0)
		{
			blueOsSignalBroadcast(queueObject->_fullSignal);
		}
		#endif //BLUE_OS_USE_SIGNAL


		// Check, if block was completely inserted
		if (copyedCount >= length)
			return copyedCount;

	#ifdef BLUE_OS_USE_SIGNAL
		if ((queueObject->_emptySignal != 0) && !(options & BLUE_OS_QUEUE_POLLING))
		{
			blueOsWait(queueObject->_emptySignal);
		}
		else
		{
			break;
		}

	}
	#endif //BLUE_OS_USE_SIGNAL

	return copyedCount;
}


BLUE_OS_QUEUE_DATA_TYPE blueOsQueueDequeueBlock(volatile BlueOsQueue* queueObject,
								uint8_t* data, BLUE_OS_QUEUE_DATA_TYPE length,
								uint8_t options)
{
	// Registers for optimization
	BLUE_OS_QUEUE_DATA_TYPE queueBegin;
	BLUE_OS_QUEUE_DATA_TYPE elementcount;
	BLUE_OS_QUEUE_DATA_TYPE copyedCount = 0;

	#ifdef BLUE_OS_USE_SIGNAL
	while (1)
	{
	#endif //BLUE_OS_USE_SIGNAL	

		uint8_t irstate = blueOsEnterCriticalSection();

		// Optimization: get local copy
		elementcount = queueObject->_elementcount;

		// Optimization: get local copy
		queueBegin = queueObject->_queueBegin;

		for ( ; (copyedCount < length) && (elementcount > 0); copyedCount++)
		{
			data[copyedCount] = queueObject->_queue[queueBegin];
	
			queueBegin++;
			
			queueBegin = queueBegin % queueObject->_queuesize;
	
			elementcount--;
		}

		// Optimization: store local copy
		queueObject->_queueBegin = queueBegin;

		// Optimization: store local copy
		queueObject->_elementcount = elementcount;	

		blueOsLeaveCriticalSection(irstate);

		#ifdef BLUE_OS_USE_SIGNAL
		if (queueObject->_emptySignal != 0)
		{
			blueOsSignalBroadcast(queueObject->_emptySignal);
		}
		#endif //BLUE_OS_USE_SIGNAL

		// Check, if block was completely inserted
		if (copyedCount >= length)
			return copyedCount;

	#ifdef BLUE_OS_USE_SIGNAL
		if ((queueObject->_fullSignal != 0) && !(options & BLUE_OS_QUEUE_POLLING))
		{
			blueOsWait(queueObject->_fullSignal);
		}
		else
		{
			break;
		}

	}
	#endif //BLUE_OS_USE_SIGNAL

	return copyedCount;
}


// </Queue>
