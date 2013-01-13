
/********************************************************************************
*Copyright (C) 2013 	Moritz Nagel 	mnagel@htwg-konstanz.de					*
*					Daniel Urbanietz 	daniel@d-urbanietz.de					*
*********************************************************************************
*FILE: blue_os_task.c															*
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

#include "blue_os_task.h"
#include "blue_os_internals.h"
#include "blue_os_debug.h"
#include "blue_os_config.h"


#ifdef BLUE_OS_USE_SHELL
	#include "blue_os_shell.h"
	#include "blue_os_shell_vt100.h"
#endif //BLUE_OS_USE_SHELL

#include <avr/interrupt.h>

// <private variables>
static uint8_t idleTaskStack[BLUE_OS_SYSTEM_STACKSIZE];
// </private variables>


// <protected variables>
volatile BlueOsTCB _idleTask;

#ifdef BLUE_OS_USE_USER_WATCHDOG
	volatile blueOsWatchdogCallbackFunction _userWatchdogCallbacks[BLUE_OS_TASK_COUNT];
	volatile uint32_t _userWatchdogTimeouts[BLUE_OS_TASK_COUNT];
	volatile uint32_t _nextUserWatchdog;
#endif //BLUE_OS_USE_USER_WATCHDOG
// </protected variables>


// <private functions>

#ifndef BLUE_OS_TASKS_NEVER_DEAD
/*
 * This is the place, where tasks are going, if they are returninng.
 */
static void blueOsFinalDestination()
{
	// Commit suicide
	// Last will: call scheduler and pass the CPU to another thread
	blueOsKill(_currentTaskId);
	// R.I.P.
}
#endif //BLUE_OS_TASKS_NEVER_DEAD

static int8_t _blueOsCreateTask(BlueOsTCB* task, uint8_t* stack, uint16_t stackSize, uint8_t priority, blueOsThread _functionPointer, int8_t slot, BlueOsTCB** handle)
{
	int8_t retval;
	int8_t i;
	uint8_t* s;

	uint8_t interruptState = blueOsEnterCriticalSection();
	
	#ifdef BLUE_OS_USE_US_DELAY
	task->_usDelay = 0;
	#endif //BLUE_OS_USE_US_DELAY

	retval = 0;
	
	uint8_t* tempStack;
	uint16_t tempStackSize;

	tempStack = stack;
	task->_stack = stack;

	tempStackSize = stackSize;
	task->_stackSize = stackSize;


	// Leave the first byte of the stack free, for the magic number
	s = (uint8_t*)(tempStack + tempStackSize - 2);

	// Push return address on stack (final destination)
	#ifndef BLUE_OS_TASKS_NEVER_DEAD
	functionPointer tempAddress = blueOsFinalDestination;
	#else
	functionPointer tempAddress = 0x00;
	#endif //BLUE_OS_TASKS_NEVER_DEAD
	
	#ifdef BLUE_OS_HW_3BYTE_PROGRAMPOINTER
	// 3-Byte program pointer
		uint32_t tempAddress2 = 0UL + (uint16_t) tempAddress; /// @todo Support for 24-bit addresses
		
		for (i = 0; i < 3; i++)
		{
			*s = ((uint8_t*)(&tempAddress2))[i];
			s--;
		}

		// Push task function address on stack
		tempAddress2 = 0UL + (uint16_t) _functionPointer; /// @todo Support for 24-bit addresses
		for (i = 0; i < 3; i++)
		{
			*s = ((uint8_t*)(&tempAddress2))[i];
			s--;
		}
	#else
	// 2-Byte program pointer
		for (i = 0; i < sizeof(functionPointer); i++)
		{
			*s = ((uint8_t*)(&tempAddress))[i];
			s--;
		}

		// Push task function address on stack
		//tempAddress = _functionPointer;
		for (i = 0; i < sizeof(blueOsThread); i++)
		{
			*s = ((uint8_t*)(&_functionPointer))[i];
			s--;
		}
	#endif

	// Initialize the registers, which are poped from the stack
	for (i = 31; i >= (uint8_t) 0; i--)
		*s-- = 0;

	// Push the initial sreg to the stack
	*s-- = 0x00;	

	task->_stackPointer = (uint8_t*)s;
	task->_priority = priority;

	retval = -1; // If this value remains -1, there was no space to insert the task
	
	if (slot < 0)
	{
		// Begin with 1, because task0 is the main application
		for (i = BLUE_OS_SYSTEM_TASKS; i < BLUE_OS_TASK_COUNT; i++)
		{
			if (_tasks[i] == 0)
			{
				retval = i;
				break;
			}
		}
	}
	else
	{
		i  = slot;
		retval = i;
	}

	if (retval >= (int8_t) 0)
	{
		// Insert the task into the next free slot
		_tasks[i] = task;

		task->_taskId = i;

		// Store magic numbers
		tempStack[tempStackSize - 1] = BLUE_OS_STACK_MAGIC_NUMBER(i);
		tempStack[0] = BLUE_OS_STACK_MAGIC_NUMBER(i);
		task->_state = BLUE_OS_STATE_ACTIVE;
		task->_handle = handle;
		if(handle)
		{
			*handle = task;
		}
		task->_fatherId = _currentTaskId;
		#ifdef BLUE_OS_USE_SIGNAL
			task->_waiting = 0;
			#ifdef BLUE_OS_USE_SEMA
				task->_owner=0;
			#endif //BLUE_OS_USE_SEMA 
		#endif //BLUE_OS_USE_SIGNAL
	}
	
	blueOsLeaveCriticalSection(interruptState);
	return retval;
}

static void blueOsIdleTask()
{
	uint8_t i;
	
	#ifdef BLUE_OS_USE_SHELL
	// Initialize shell
	blueOsInitShell();
		#ifdef BLUE_OS_USE_SHELL_VT100
		blueOsInitShellVt100();
		#endif
	blueOsWriteStringFlash("BlueOS is started.\r\n");
	#endif

	// Idle task should be the last to execute
	blueOsYield();

	#ifdef BLUE_OS_IMPLEMENT_OWN_IDLE_TASK
	// Call user specific idle task
	blueOsOwnIdleTask();

	#else
	// Standard idle task
	while (1)
	{
		// Ignore idle task, as it always is active!
		for (i = 1; i < BLUE_OS_TASK_COUNT; i++)
		{
			if ((_tasks[i] != 0) && (_tasks[i]->_state == BLUE_OS_STATE_ACTIVE))
				blueOsYield();
		}
	}
	#endif

	#ifdef BLUE_OS_DEBUG
	// The idle task must not return!
	blueOsKernelPanic(BLUE_OS_PANIC_IDLE_TASK_RETURNED);
	#endif
}

// </private functions>

int8_t blueOsCreateTask(BlueOsTCB* task, uint8_t* stack, uint16_t stackSize, uint8_t priority, blueOsThread _functionPointer, BlueOsTCB** handle)
{
	return _blueOsCreateTask(task, stack, stackSize, priority, _functionPointer, -1,handle);
}

#ifdef BLUE_OS_DYNAMIC_TASK
int8_t blueOsCreateDynamicTask(uint16_t stackSize, uint8_t priority, blueOsThread _functionPointer, BlueOsTCB** handle)
{
	uint8_t* mem;
	if(blue_os_alloc(&mem, stackSize + sizeof(BlueOsTCB)))
		return -1;

	uint8_t crit = blueOsEnterCriticalSection();
	if(_blueOsCreateTask((BlueOsTCB*)mem, mem+sizeof(BlueOsTCB), stackSize, priority, _functionPointer, -1,handle) == -1)
	{
		blue_os_free(mem);
		blueOsLeaveCriticalSection(crit);
		return -1;	
	}
	blue_os_memChown(mem, ((BlueOsTCB*)mem)->_taskId);
	blueOsLeaveCriticalSection(crit);

	return 0;

}	
#endif //BLUE_OS_DYNAMIC_TASK

void blueOsYield()
{
	blueOsSoftTimerInterrupt();
}

#ifndef BLUE_OS_TASKS_NEVER_DEAD
void blueOsKill(int8_t taskId)
{
	volatile uint8_t i;

	#ifdef BLUE_OS_USE_SIGNAL
	BlueOsTCB* nextWaiting;
	#endif //BLUE_OS_USE_SIGNAL

	#ifdef BLUE_OS_USE_SEMA
	BlueOsSema* mySema;
	#endif //BLUE_OS_USE_SEMA 

	cli();

	uint8_t kill = 0;
	
	//Kill the complete task-family except system tasks
	for(i=BLUE_OS_SYSTEM_TASKS;i<BLUE_OS_TASK_COUNT;i++)
	{
		if(_tasks[i]==0)
			continue;
		//check for family dependencies
		kill = _tasks[i]->_fatherId;
		while(kill != 0)
		{
			if(kill == taskId)
			{
				break;
			}		
			kill = _tasks[kill]->_fatherId;		
		}
		if((!kill && i !=taskId) || _tasks[i]->_state == BLUE_OS_STATE_DEAD)
			continue;

		// Not allowed, to kill idle task
		if ((i >= (int8_t) BLUE_OS_SYSTEM_TASKS) && (i < (int8_t) BLUE_OS_TASK_COUNT))
		{
			// Remove task from scheduler
			_tasks[i]->_state = BLUE_OS_STATE_DEAD;
			
			//Release all ressources
				#ifdef BLUE_OS_USE_SIGNAL
					if(_tasks[i]->_waiting)
					{
						if(_tasks[i]->_waiting->_firstWaitingTask == _tasks[i]->_waiting->_lastWaitingTask)
						{
							_tasks[i]->_waiting->_firstWaitingTask = 0;
							_tasks[i]->_waiting->_lastWaitingTask = 0;
						}
						else
						{
							nextWaiting = _tasks[i]->_waiting->_firstWaitingTask;
							if(nextWaiting == _tasks[i])
							{
								_tasks[i]->_waiting->_firstWaitingTask = nextWaiting->_nextWaitingTask;
							}
							else
							{
								while(nextWaiting)
								{
									if(nextWaiting->_nextWaitingTask == _tasks[i])
									{
										if(nextWaiting->_nextWaitingTask == _tasks[i]->_waiting->_lastWaitingTask)
										{
											_tasks[i]->_waiting->_lastWaitingTask = nextWaiting;
										}
										else
										{
											nextWaiting->_nextWaitingTask = _tasks[i]->_nextWaitingTask;
										} 
										break;
									}
								}
							}
						}	
					}
					
				#endif //BLUE_OS_USE_SIGNAL
		}
		else
		{
			blueOsKernelPanic(BLUE_OS_PANIC_IDLE_TASK_RETURNED);
		}
	}
	for(i=0;i<BLUE_OS_TASK_COUNT;i++)
	{
		if(_tasks[i] && _tasks[i]->_state == BLUE_OS_STATE_DEAD)
		{
			#ifdef BLUE_OS_USE_SEMA
				while((mySema = _tasks[i]->_owner) != 0)
				{
					_tasks[i]->_owner = mySema->next;
					mySema->next = 0;
					mySema->owner = 0;
					blueOsSignalSingle(&(mySema->sig)); 
				}
			#endif //BLUE_OS_USE_SEMA 

			#ifdef BLUE_OS_MEMORY
				#ifndef BLUE_OS_TASKS_NEVER_DEAD
					blue_os_freeKill(_tasks[i]->_taskId);
				#endif
			#endif //BLUE_OS_MEMORY

			if(_tasks[i]->_handle)
			{
				*(_tasks[i]->_handle) = 0;
				_tasks[i]->_handle = 0;
			}
			_tasks[i] = 0;
		}
	}

	// If a task kills itself, it will stop existing now
	// => no zombie is created
	blueOsSoftTimerInterrupt();

	/******************
	 *     ______     *
 	 *    /      \    *
	 *   /        \   *
	 *  |  R.I.P.  |  *
	 *  |          |  *
	 *  |    ##    |  *
	 *  |  ######  |  *
	 *   | ###### |   *
	 *   |   ##   |   *
	 *   |   ##   |   *
	 *   |        |   *
	 *    |      |    *
	 *    |      |    *
	 *    |      |    *
	 *     \____/     *
	 *                *
	 ******************/
}
#endif //BLUE_OS_TASKS_NEVER_DEAD

void blueOsStart()
{
	_currentTask = &_idleTask;
	_currentTaskId = 0;

	// Create idle task
	_blueOsCreateTask(_currentTask, idleTaskStack, BLUE_OS_SYSTEM_STACKSIZE, 0xff, blueOsIdleTask, 0, 0);

	// Set stack pointer to the bottom of the stack of the idle task
	SP = (uint16_t)(idleTaskStack + (BLUE_OS_SYSTEM_STACKSIZE - 1 - 2 -2 -1));
	
	// Now directly call the idle task
	// Interrupts will be enabled there
	blueOsIdleTask();
}

#ifdef BLUE_OS_USE_USER_WATCHDOG
int8_t blueOsUserWatchdog(uint32_t timeoutMs, blueOsWatchdogCallbackFunction functionPointer, uint8_t taskId)
{
	if (taskId >= BLUE_OS_TASK_COUNT)
		return -1; // Invalid task ID

	uint8_t irstate = blueOsEnterCriticalSection();
	
	uint32_t tempTimeoutMs = _milliseconds + timeoutMs;

	_userWatchdogCallbacks[taskId] = functionPointer;
	_userWatchdogTimeouts[taskId] = tempTimeoutMs;

	// Update the global timeout
	if (timeoutMs < _nextUserWatchdog)
		_nextUserWatchdog = tempTimeoutMs;
	
	blueOsLeaveCriticalSection(irstate);

	return 0;
}

inline void blueOsUserWatchdogSchedule()
{
	int8_t nextTask = -1;
	uint32_t smallestTimeout = TIMER_TIMEOUT_INACTIVE;

	uint8_t i;
	
	// Call all timed out callback functions
	for (i = 0; i < BLUE_OS_TASK_COUNT; i++)
	{
		if (_userWatchdogTimeouts[i] <= _milliseconds)
		{
			// Reset timeout
			_userWatchdogTimeouts[i] = TIMER_TIMEOUT_INACTIVE;

			// Slot timed out, call callback function
			(_userWatchdogCallbacks[i])();
		}

		// Look for smallest timeout
		if (_userWatchdogTimeouts[i] < smallestTimeout)
		{
			smallestTimeout = _userWatchdogTimeouts[i];
			nextTask = i;
		}
	}

	// Set next timeout
	if (nextTask >= 0)
	{
		_nextUserWatchdog = _userWatchdogTimeouts[nextTask];
	}
	else
	{
		_nextUserWatchdog = TIMER_TIMEOUT_INACTIVE; // Timeout inactive
	}
}
#endif //BLUE_OS_USE_USER_WATCHDOG

