
/********************************************************************************
*Copyright (C) 2013 	Moritz Nagel 	mnagel@htwg-konstanz.de					*
*					Daniel Urbanietz 	daniel@d-urbanietz.de					*
*********************************************************************************
*FILE: blue_os_task.h															*
*																				*
*AUTHOR(S): M. Nagel, D. Urbanietz													*
*																				*
*DESCRIPTION:																	*
*This file is part of the blueOs kernel.										*
*It contains the task definition and some the most important task functions		*
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


#ifndef BLUE_OS_TASK_H
#define BLUE_OS_TASK_H

#include "blue_os_config.h"
#include "blue_os_synchronization.h"

#include <avr/io.h>

// Defines

/**
 * If the task state is BLUE_OS_STATE_ACTIVE, the scheduler
 * will schedule the task.
 */
#define BLUE_OS_STATE_ACTIVE	2

/**
 * If the task state is BLUE_OS_STATE_WAITING, the scheduler
 * will not schedule the task.
 */
#define BLUE_OS_STATE_WAITING	1

/**
 * If the task state is BLUE_OS_STATE_DEAD, the task is
 * removed from the schedule queue and won't be scheduled.
 */
#define BLUE_OS_STATE_DEAD		0

/**
 * This makro calculates the magic numbers for the stack
 * for a specific task.
 */
#define BLUE_OS_STACK_MAGIC_NUMBER(n) ~n


// Typedefs

typedef void (*blueOsThread)();

typedef blueOsThread functionPointer;

typedef volatile struct BlueOsTCB
{
	volatile uint8_t* _stackPointer;
	
	volatile uint8_t _state;

	volatile uint8_t _priority;
	
	volatile uint8_t* _stack;
	volatile uint16_t _stackSize;

	volatile uint8_t _taskId;
	volatile uint8_t _fatherId;

	volatile struct BlueOsTCB** _handle;

	#ifdef BLUE_OS_USE_SIGNAL
		volatile struct BlueOsTCB* _nextWaitingTask;
		volatile BlueOsSignal* _waiting;
		#ifdef BLUE_OS_USE_SEMA
			volatile BlueOsSema* _owner;
		#endif //BLUE_OS_USE_SEMA 
	#endif //BLUE_OS_USE_SIGNAL

	#ifdef BLUE_OS_USE_US_DELAY
	volatile uint16_t _usDelay;
	#endif //BLUE_OS_USE_US_DELAY
} BlueOsTCB;

typedef BlueOsTCB* BlueOsTaskHandle; 

// Functions

/**
 * This function adds a task to the scheduler and initializes the stack and
 * data structures.
 *
 * @param task Pointer to the task structure.
 * @param stack Pointer to the stack of the task.
 * @param stackSize Size of the stack.
 * @param priority Priority of the task. 0: Highest priority; 255: Lowest priority
 * @param _functionPointer Function of the task.
 *
 * @return	 0: Task was added to the scheduler \n
 *			-1: Could not add task to the scheduler
 */
int8_t blueOsCreateTask(BlueOsTCB* task, uint8_t* stack, uint16_t stackSize, uint8_t priority, blueOsThread _functionPointer, BlueOsTCB** handle);


#ifdef BLUE_OS_DYNAMIC_TASK
	int8_t blueOsCreateDynamicTask(uint16_t stackSize, uint8_t priority, blueOsThread _functionPointer, BlueOsTCB** handle);
#endif


/**
 * Passes the CPU to another task, but stays active.
 *
 * This function does a context switch.
 */
void blueOsYield();

#ifndef BLUE_OS_TASKS_NEVER_DEAD
/**
 * Removes a task from the scheduler, and sets its state to: BLUE_OS_STATE_DEAD.
 *
 * This function does a context switch.
 *
 * @param taskId ID of the task, which will be removed from the scheduler.
 */
void blueOsKill(int8_t taskId);
#endif //BLUE_OS_TASKS_NEVER_DEAD

#ifdef BLUE_OS_USE_DELAY
/**
 * Sets the current task to sleeping mode for a specified time.
 * 
 * This function does a context switch.
 *
 * @param delayMs Time in milliseconds, how long the task will sleep.
 */
void blueOsDelay(uint16_t delayMs);

#ifdef BLUE_OS_USE_US_DELAY
/**
 * Sets the current task to sleeping mode for a specified time.
 * 
 * This function does a context switch.
 *
 * @param delayUs Time in microseconds, how long the task will sleep.
 */
void blueOsDelayUs(uint16_t delayUs);
#endif //BLUE_OS_USE_US_DELAY
#endif //BLUE_OS_USE_DELAY

/**
 * If BLUE_OS_IMPLEMENT_OWN_IDLE_TASK is defined, blueOsOwnIdleTask();
 * can be implemented and will be called by the kernel.
 */
#ifdef BLUE_OS_IMPLEMENT_OWN_IDLE_TASK
void blueOsOwnIdleTask();
#endif //BLUE_OS_IMPLEMENT_OWN_IDLE_TASK

/**
 * This function starts the kernel. It passes the control to the
 * idle task.
 *
 * @warning This function does never return
 * @warning The main stack will be disabled
 */
void blueOsStart();


#ifdef BLUE_OS_USE_USER_WATCHDOG
/*
 * This type is the callback function type.
 */
typedef void (*blueOsWatchdogCallbackFunction)();

/**
 * This function registers a callback function to the given task.
 * This callback function will be called, if a timeout is reached.
 *
 * @param timeoutMs Delta time, when the callback function will be called.
 * @param functionPointer Function, which is called on timeout.
 * @param taskId ID of the task, for which this timeout will be registered.
 *
 * @return 0: OK; -1: Invalid task ID
 */
int8_t blueOsUserWatchdog(uint32_t timeoutMs, blueOsWatchdogCallbackFunction functionPointer, uint8_t taskId);
#endif //BLUE_OS_USE_USER_WATCHDOG

#endif
