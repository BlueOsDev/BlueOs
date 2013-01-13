/********************************************************************************
*Copyright (C) 2013 	Moritz Nagel 	mnagel@htwg-konstanz.de					*
*					Daniel Urbanietz 	daniel@d-urbanietz.de					*
*********************************************************************************
*FILE: blue_os_synchronization.c												*
*																				*
*AUTHOR(S): M.Nagel, D.Urbanietz												*
*																				*
*DESCRIPTION:																	*
*This module provides the sychronization mechanism. You can configure it in the	*
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

#include "blue_os_config.h"
#include "blue_os_synchronization.h"
#include "blue_os_container.h"
#include "blue_os_internals.h"



#include <avr/interrupt.h>

#ifdef BLUE_OS_USE_SIGNAL

// <Signal-Wait>
void blueOsInitSignal(BlueOsSignal* signal)
{
	signal->_firstWaitingTask = 0;
	signal->_lastWaitingTask = 0;
	signal->_callBack = 0;
}

int8_t blueOsSignalSingle(BlueOsSignal* signal)
{
	int8_t retval = -1;
	
	// Only set the task active, if the task should switch, use yield()
	uint8_t irstate = blueOsEnterCriticalSection();

	BlueOsTCB* sleepingTask = signal->_firstWaitingTask;
	BlueOsTCB* lastTask = 0;
	while(1)
	{
		if (sleepingTask != 0)
		{
			if(sleepingTask->_state == BLUE_OS_STATE_WAITING)
			{
				retval = 0;

				// Wake first task
				sleepingTask->_state = BLUE_OS_STATE_ACTIVE;
				sleepingTask->_waiting = 0;

				// Remove task
				signal->_firstWaitingTask = sleepingTask->_nextWaitingTask;

				// Check, if the end needs to be removed, too
				if (signal->_firstWaitingTask == 0)
					signal->_lastWaitingTask = 0;

				// Dequeue
				if (lastTask != 0)
				{
					lastTask->_nextWaitingTask = sleepingTask->_nextWaitingTask;
				}
				else
				{
					signal->_firstWaitingTask = sleepingTask->_nextWaitingTask;
				}

				break;
			}

			// This task is not wakeable, try the next one
			lastTask = sleepingTask;
			sleepingTask = sleepingTask->_nextWaitingTask;
		}
		else
			break;
	}
	blueOsLeaveCriticalSection(irstate);

	return retval;
}

void blueOsSignalBroadcast(BlueOsSignal* signal)
{
	while (blueOsSignalSingle(signal) >= 0)
		;
}

int8_t blueOsWait(BlueOsSignal* signal)
{
	int8_t retval = 0;
	BlueOsCallBack* tmp;
	uint8_t irstate = blueOsEnterCriticalSection();

	// Check, if there is already a task waiting
	if (signal->_firstWaitingTask == 0)
	{
		signal->_firstWaitingTask = _currentTask;
		signal->_lastWaitingTask = _currentTask;
	}
	else
	{
		signal->_lastWaitingTask->_nextWaitingTask = _currentTask;
		signal->_lastWaitingTask = _currentTask;
	}

	_currentTask->_nextWaitingTask = 0;
	_currentTask->_state = BLUE_OS_STATE_WAITING;
	_currentTask->_waiting = signal;

	
	//execute callback functions
	tmp = signal->_callBack;
	while(tmp)
	{
		tmp->func((uint8_t*)signal,_currentTaskId);
		tmp = tmp->next;
	}
	
	// Call the scheduler, because this task should be sleeping now
	blueOsSoftTimerInterrupt();
	
	blueOsLeaveCriticalSection(irstate);

	return retval;
}


void blueOsSignalAddCallBack(BlueOsSignal* signal, BlueOsCallBack* callBack )
{
	uint8_t irstate = blueOsEnterCriticalSection();
	callBack->next = signal->_callBack;
	signal->_callBack = callBack;
	blueOsLeaveCriticalSection(irstate);
}

void blueOsSignalRemoveCallBack(BlueOsSignal* signal, BlueOsCallBack* callBack)
{
	uint8_t irstate = blueOsEnterCriticalSection();
	BlueOsCallBack* tmp = signal->_callBack;
	if(tmp == callBack)
		 signal->_callBack = callBack->next;
	while(tmp)
	{
		if(tmp->next == callBack)
		{ 
			signal->_callBack = callBack->next;
			break;
		}
		tmp = tmp->next;		
	}
	blueOsLeaveCriticalSection(irstate);
}

// </Signal-Wait>

#endif


#ifdef BLUE_OS_USE_SEMA

/**
 * Intialize the semaphore	
 */
int8_t blueOsInitSema(BlueOsSema *sema)
{
	
	uint8_t crit = blueOsEnterCriticalSection();
	blueOsInitSignal(&(sema->sig));
	sema->owner = 0; //idle Task
	sema->next = 0;
	blueOsLeaveCriticalSection(crit);
	return 0;
}

int8_t blueOsAcquireSema(BlueOsSema *sema)
{
	uint8_t crit = blueOsEnterCriticalSection();
	if(sema->owner > 0)
	{	
		blueOsWait(&(sema->sig));
	}
	sema->owner = _currentTaskId;
	if(_currentTask->_owner)
	{
		BlueOsSema* tmp = _currentTask->_owner;
		while(tmp->next!=0)
		{
			tmp = tmp->next;
		}
		tmp->next = sema;
	}
	else
	{
		_currentTask->_owner = sema;
	}
	sema->next = 0;
	
	blueOsLeaveCriticalSection(crit);
	return 0;
}
int8_t blueOsReleaseSema(BlueOsSema *sema)
{
	uint8_t crit = blueOsEnterCriticalSection();
	if(sema->owner == _currentTaskId)
	{
		blueOsSignalSingle(&(sema->sig));
		BlueOsSema* tmp = _currentTask->_owner;

		if(tmp==sema)
		{
			if(tmp->next)
				_currentTask->_owner=tmp->next;
		}
		else
		{
			while(tmp->next!=sema)
			{
				tmp = tmp->next;
			}
			tmp->next = sema->next;
		}	
		
		sema->owner = 0;
	}
	blueOsLeaveCriticalSection(crit);
	return 0;
}

#endif //BLUE_OS_USE_SEMA

#ifdef BLUE_OS_USE_EVENT
int8_t blue_os_eventInit(BlueOsEvent *event, uint8_t dataSize,blueOsEventOwnListen list)
{
	uint8_t i;
	uint8_t crit_state = blueOsEnterCriticalSection();
	for(i=0;i<BLUE_OS_MAX_TASKS ;i++)
	{
		event->data[i] = 0;
		event->mask[i] = 0;
		blueOsInitSignal(&(event->sig[i]));
	}
	event->d_size = dataSize;
	event->_list = list; 
	blueOsLeaveCriticalSection(crit_state);
	return 0;
}

int8_t blue_os_eventListen(BlueOsEvent *event,uint8_t mask, uint8_t *data)
{
	uint8_t crit_state = blueOsEnterCriticalSection();
	int8_t id = _currentTaskId - BLUE_OS_SYSTEM_TASKS;
	event->data[id] = data;
	event->mask[id] = mask;
	
	if(event->_list != 0)
		event->_list();	
				
	if(blueOsWait(&(event->sig[id])) < 0)
	{
		blueOsLeaveCriticalSection(crit_state);
		event->data[id] = 0;
		return -1;
	};
	blueOsLeaveCriticalSection(crit_state);
	return 0;
}
int8_t blue_os_eventSignal(BlueOsEvent *event, uint8_t *data)
{
	uint8_t i, j;
	uint8_t ret = 0;
	uint8_t crit_state = blueOsEnterCriticalSection();
	for(i=0;i<BLUE_OS_MAX_TASKS;i++)
	{
		if(event->data[i] != 0)
		{
			if(event->mask[i] != 0 && event->mask[i] != data[0])
			{
				continue;
			}
			for(j=0;j<event->d_size;j++)
			{
				event->data[i][j] = data[j];
			}
			blueOsSignalSingle(&event->sig[i]);
			event->data[i] = 0;
			ret++;
		}
	}
	blueOsLeaveCriticalSection(crit_state);
	return ret;
}

int8_t blue_os_eventGetListenerCnt(BlueOsEvent *event, uint8_t mask)
{
	uint8_t i;
	uint8_t ret = 0;
	uint8_t crit_state = blueOsEnterCriticalSection();
	for(i=0;i<BLUE_OS_MAX_TASKS;i++)
	{
		if(event->data[i] != 0 && (event->mask[i] == mask || mask == 0))
			ret++;
	}
	blueOsLeaveCriticalSection(crit_state);
	return ret;
}

#endif //BLUE_OS_USE_EVENT

// <Critical section>
uint8_t blueOsEnterCriticalSection()
{
	uint8_t state = SREG & 0x80;
	cli();
	return state;
}

void blueOsLeaveCriticalSection(uint8_t state)
{
	if(state & 0x80)
		sei();
	else
		cli();
}
// </Critical section>
