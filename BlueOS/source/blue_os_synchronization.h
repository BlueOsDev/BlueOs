
/********************************************************************************
*Copyright (C) 2013 	Moritz Nagel 	mnagel@htwg-konstanz.de					*
*					Daniel Urbanietz 	daniel@d-urbanietz.de					*
*********************************************************************************
*FILE: blue_os_synchronization.h												*
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


#ifndef BLUE_OS_SYNCHRONIZATION_H
#define BLUE_OS_SYNCHRONIZATION_H

#include "blue_os_config.h"
#include "blue_os_callBack.h"

#include <avr/io.h>


// Typedefs

#ifdef BLUE_OS_USE_SIGNAL

typedef struct BlueOsSignal
{
	volatile struct BlueOsTCB* _firstWaitingTask;
	volatile struct BlueOsTCB* _lastWaitingTask;
	volatile BlueOsCallBack* _callBack;
} BlueOsSignal;

#endif // BLUE_OS_USE_SIGNAL

#ifdef BLUE_OS_USE_SEMA

#ifndef BLUE_OS_USE_SIGNAL
#error "You have to enable signal, to use the semaphore"
#endif

typedef volatile struct BlueOsSema
{
	BlueOsSignal sig;
	uint8_t owner;
	volatile struct BlueOsSema* next;

}BlueOsSema;

#endif

#ifdef BLUE_OS_USE_EVENT
	#ifndef BLUE_OS_USE_SIGNAL
	#error "You have to enable signal, to use the Event-Interface"
	#endif
typedef void (*blueOsEventOwnListen)();
typedef struct BlueOsEvent
{
	uint8_t* data[BLUE_OS_MAX_TASKS];
	uint8_t  mask[BLUE_OS_MAX_TASKS];
	uint8_t  d_size;
	BlueOsSignal sig[BLUE_OS_MAX_TASKS];
	blueOsEventOwnListen _list;
} BlueOsEvent;

int8_t blue_os_eventListen(BlueOsEvent *event,uint8_t mask, uint8_t *data);
int8_t blue_os_eventSignal(BlueOsEvent *event, uint8_t *data);
int8_t blue_os_eventInit(BlueOsEvent *event, uint8_t dataSize,blueOsEventOwnListen list);
int8_t blue_os_eventGetListenerCnt(BlueOsEvent *event, uint8_t mask);
#endif //BLUE_OS_USE_EVENT


// Functions

/**
 * Stores if the interrupts are currently enabled and disables the interrupts.
 *
 * @return This function returns the interrupt state before the function call.
 */
uint8_t blueOsEnterCriticalSection();

/**
 * Resores the interrupt state when blueOsEnterCriticalSection() was called
 * the last time.
 *
 * @param state This is the interrupt state, that will be restored. (return value of
 * blueOsEnterCriticalSection())
 */
void blueOsLeaveCriticalSection(uint8_t state);


#ifdef BLUE_OS_USE_SIGNAL
/**
 * This function initializes a signal structure.
 *
 * @param signal Pointer to the signal structure.
 */
void blueOsInitSignal(BlueOsSignal* signal);

/**
 * This function wakes a single task, which is waiting for a signal.
 *
 * @waning This function does NO context switch. If you need one, call 
 * blueOsYield() after this function.
 *
 * @param signal Pointer to the signal, where a task will be waken.
 *
 * @return 	 0: Task was set active.
 *			-1: There was no task to wake up.
 */
int8_t blueOsSignalSingle(BlueOsSignal* signal);

/**
 * This function wakes all tasks, which are waiting for a signal.
 *
 * @waning This function does NO context switch. If you need one, call 
 * blueOsYield() after this function.
 *
 * @param signal Pointer to the signal, where the tasks will be waken.
 */
void blueOsSignalBroadcast(BlueOsSignal* signal);

/**
 * This function sets the current task into sleeping mode. The task will
 * be reactivated, when the signal is set.
 * 
 * This function does a context switch, if successful (see return values).
 *
 * @param signal Pointer to the signal, the task will wait on.
 *
 * @return	 0: The task was sleeping and has been woken now.
 *			-1: The task could not be set to sleeping mode. (no context switch)
 */
int8_t blueOsWait(BlueOsSignal* signal);


void blueOsSignalAddCallBack(BlueOsSignal* signal, BlueOsCallBack* callBack );
void blueOsSignalRemoveCallBack(BlueOsSignal* signal, BlueOsCallBack* callBack);


#endif //BLUE_OS_USE_SIGNAL

#ifdef BLUE_OS_USE_SEMA


/**
 * Intialize the semaphore	
 */
int8_t blueOsInitSema(BlueOsSema *sema);
int8_t blueOsAcquireSema(BlueOsSema *sema);
int8_t blueOsReleaseSema(BlueOsSema *sema);
#ifdef BLUE_OS_SEMA_MONITORING
int8_t blueOsCheckSemas();
int8_t blueOsRepairSemas();
#endif //BLUE_OS_SEMA_MONITORING


#endif //BLUE_OS_USE_SEMA

#endif
