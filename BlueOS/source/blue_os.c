/********************************************************************************
*Copyright (C) 2013 	Moritz Nagel 	mnagel@htwg-konstanz.de					*
*					Daniel Urbanietz 	daniel@d-urbanietz.de					*
*********************************************************************************
*FILE: blue_os.c																*
*																				*
*AUTHOR(S): M. Nagel, D. Urbanietz												*
*																				*
*DESCRIPTION:																	*
*This module contains the scheduler and dispatcher of the BlueOS				*
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

#include "blue_os.h"
#include "blue_os_config.h"
#include "blue_os_hardware.h"
#include "blue_os_task.h"
#include "blue_os_synchronization.h"
#include "blue_os_internals.h"
#include "blue_os_debug.h"

#include <avr/interrupt.h>
#include <avr/io.h>

// <definitions>
#define PUSH(r) asm volatile("push r"#r);
#define POP(r) asm volatile("pop r"#r);

// </definitions>

// <private variables>
static volatile uint8_t _isSoftInterrupt;

#ifdef BLUE_OS_USE_DELAY
static volatile uint32_t _nextDelayTimeout;

static volatile uint32_t _taskDelays[BLUE_OS_TASK_COUNT];

#ifdef BLUE_OS_USE_US_DELAY
static int8_t _currentUsDelayTask;
#endif //BLUE_OS_USE_US_DELAY
#endif //BLUE_OS_USE_DELAY
// </private variables>


// <protected variables>

// This variable holds the index of the _currentTask task,
// so thre round robin is simply a ++
volatile int8_t _currentTaskId;

volatile BlueOsTCB* _currentTask;

volatile BlueOsTCB* _tasks[BLUE_OS_TASK_COUNT];

volatile uint32_t _milliseconds;
// </protected variables>


// <private functions>
void BLUE_OS_INTERRUPT() __attribute__ ( ( signal, naked ) );


#ifdef BLUE_OS_SCHEDULER_PRIO_RR
static uint8_t blueOsSchedulePrioRR()
{
	uint16_t selectedPriority; // Use a 16 bit value, that 0xff can also be selected

	uint8_t temp;

	uint8_t i;

	BlueOsTCB* tempTCB;

	uint8_t selectedTask;

	selectedPriority = 0xffff;
	temp = _currentTaskId;
	selectedTask = 0;

	// Priority based round robin
	for (i = 0; i < (uint8_t)BLUE_OS_TASK_COUNT; i++)
	{
		temp++;
		if (temp >= (uint8_t)BLUE_OS_TASK_COUNT)
			temp = 0;

		tempTCB = _tasks[temp];
		if (tempTCB != 0)
		{
			if ((tempTCB->_priority < selectedPriority)
				&& (tempTCB->_state == (uint8_t)BLUE_OS_STATE_ACTIVE))
			{
				selectedPriority = tempTCB->_priority;
				selectedTask = temp;
			}
		}
	}

	return selectedTask;
}
#endif //BLUE_OS_SCHEDULER_PRIO_RR



#ifdef BLUE_OS_USE_DELAY
/*
 * Wakes all tasks, whose delay has timed out and schedules the next one
 */
static void blueOsDelaySchedule()
{
	// Call all timed out callback functions
	// And select the next timeout
	int8_t nextSlot = -1;
	uint32_t smallestTimeout = TIMER_TIMEOUT_INACTIVE;

	register uint32_t tempDelay asm("r8");
	
	#ifdef BLUE_OS_USE_US_DELAY
	uint16_t smallestUs = 1001;
	uint16_t tempUs;
	int8_t nextUsSlot = -1;
	#endif //BLUE_OS_USE_US_DELAY

	uint8_t i = BLUE_OS_TASK_COUNT;

	/*
	 * Jede Task bekommt zu den millisekunden noch
	 * mikrosekunden. Der scheduler für die millisekunden
	 * bleibt gleich, aber wenn es mehrere tasks mit der
	 * gleichen millisekunde gibt, werden die mikrosekunden
	 * unterschieden, allerdings nur, wenn die verbleibende zeit
	 * kleiner ist als 1ms.
	 *
	 * Wenn es mikrosekundentimer gibt, und der task in der
	 * aktuellen mikrosekunde aktiv wird, wird der Timer extra
	 * so gesetzt, dass die spezielle Zeit erreicht wird.
	 * In dem Fall werden bei einem interrupt die millisekunden
	 * nicht inkrementiert, erst wenn die summer der mikrosekunden
	 * wieder eine Millisekunde ergibt.
	 * 
	 */
	do
	{
		--i; // Optimazion

		tempDelay = _taskDelays[i];

		// Wake the task, if needed
		if (tempDelay <= _milliseconds)
		{
			#ifdef BLUE_OS_USE_US_DELAY
			tempUs = _tasks[i]->_usDelay;
			if (tempUs)
			{
				// Task has microseconds to wait, store compare value
				if (tempUs < smallestUs)
				{
					smallestUs = tempUs;
					nextUsSlot = i;

					// Timeout is reseted in the compare ISR!
				}
			}
			else
			{
			#endif //BLUE_OS_USE_US_DELAY

			// Delay Complete, wake task
			_tasks[i]->_state = BLUE_OS_STATE_ACTIVE;

			// Reset timeout
			tempDelay = TIMER_TIMEOUT_INACTIVE;

			_taskDelays[i] = tempDelay;

			#ifdef BLUE_OS_USE_US_DELAY
			}
			#endif //BLUE_OS_USE_US_DELAY
		}

		// Check, if this could be the next timeout
		if (tempDelay < smallestTimeout)
		{

			smallestTimeout =tempDelay;
			nextSlot = i;
		}
	}
	while (i != 0);

	_nextDelayTimeout = smallestTimeout;

	#ifdef BLUE_OS_USE_US_DELAY
	// Now register the US compare value
	if (nextUsSlot >= 0)
	{
		_currentUsDelayTask = nextUsSlot;
		TIMER_COMPARE_VALUE = TIMER_COUNTER_INIT_VALUE + 
								((TIMER_TICKS_MS*smallestUs)/1000UL);
		TIMER_ENABLE_COMPARE;

		if (TIMER_COUNTER >= TIMER_COMPARE_VALUE)
		{
			TIMER_COMPARE_VALUE = TIMER_COUNTER;
		}
	}
	#endif //BLUE_OS_USE_US_DELAY
}
#endif //BLUE_OS_USE_DELAY


#ifdef BLUE_OS_USE_US_DELAY
ISR(TIMER_COMPARE_INTERRUPT)
{
	uint8_t i = BLUE_OS_TASK_COUNT;

	uint16_t currentUs = blueOsGetUs();

	uint16_t smallestUs = 1001;
	uint16_t tempUs;
	int8_t nextUsSlot = -1;

	// Wake all needed tasks
	do
	{
		--i; // Optimazion

		// Wake the task, if needed
		if (_taskDelays[i] <= _milliseconds)
		{
			tempUs = _tasks[i]->_usDelay;

			if (tempUs <= currentUs)
			{
				// Delay Complete, wake task
				_tasks[i]->_state = BLUE_OS_STATE_ACTIVE;

				// Reset timeout
				_taskDelays[i] = TIMER_TIMEOUT_INACTIVE;

				_tasks[i]->_usDelay = 0;
			}
			else
			{
				// Task must be waken later
				if (tempUs < smallestUs)
				{
					smallestUs = tempUs;
					nextUsSlot = i;

					// Timeout is reseted in the compare ISR!
				}
			}
		}
	}
	while (i != 0);

	if (nextUsSlot >= 0)
	{
		_currentUsDelayTask = nextUsSlot;
		TIMER_COMPARE_VALUE = TIMER_COUNTER_INIT_VALUE + 
								((TIMER_TICKS_MS*smallestUs)/1000UL);
		TIMER_ENABLE_COMPARE;

		if (TIMER_COUNTER >= TIMER_COMPARE_VALUE)
		{
			TIMER_COMPARE_VALUE = TIMER_COUNTER;
		}
	}
	else
	{
		// All US timers done
		_currentUsDelayTask = -1;

		TIMER_DISABLE_COMPARE;
	}

	// Do context switch
	// Don't call the time functions in the ISR
	_isSoftInterrupt = 1;

	BLUE_OS_INTERRUPT();
}
#endif //BLUE_OS_USE_US_DELAY
// </private functions>


// <protected functions>
void blueOsSoftTimerInterrupt()
{
	cli();

	_isSoftInterrupt = 1;
	
	BLUE_OS_INTERRUPT();
}
// </protected functions>


void blueOsInit()
{	
	_isSoftInterrupt = 0;

	_currentTaskId = 0; // Idle task
	
	//for (uint8_t i = 0; i < (uint8_t) BLUE_OS_TASK_COUNT; i++)
	uint8_t i = BLUE_OS_TASK_COUNT;

	do
	{
		--i; // Optimazion

		_tasks[i] = 0;

		#ifdef BLUE_OS_USE_DELAY
		_taskDelays[i] = TIMER_TIMEOUT_INACTIVE;
		#endif //BLUE_OS_USE_DELAY

		#ifdef BLUE_OS_USE_USER_WATCHDOG
			_userWatchdogTimeouts[i] = TIMER_TIMEOUT_INACTIVE;
		#endif //BLUE_OS_USE_USER_WATCHDOG
	}
	while (i != 0);

	#ifdef BLUE_OS_USE_DELAY
	_nextDelayTimeout = TIMER_TIMEOUT_INACTIVE;
	#ifdef BLUE_OS_USE_US_DELAY
	_currentUsDelayTask = -1;
	#endif //BLUE_OS_USE_US_DELAY
	#endif //BLUE_OS_USE_DELAY

	#ifdef BLUE_OS_USE_USER_WATCHDOG
			_nextUserWatchdog = TIMER_TIMEOUT_INACTIVE;
	#endif //BLUE_OS_USE_USER_WATCHDOG

	#ifdef BLUE_OS_MEMORY
			blue_os_mem_init();
	#endif 
	
	_milliseconds = 0;

	// Normal operation mode, no waveform
	// Select prescaler 1/64
	TIMER_CONGFIGURATION;

	// Count TIMER_TICKS_MS times (1 ms) until the overflow interrupt is called
	TIMER_COUNTER = TIMER_COUNTER_INIT_VALUE;

	// Enable timer overflow interrupt
	TIMER_INTERRUPT_ENABLE;
}


ISR(BLUE_OS_INTERRUPT)
{
	#ifdef BLUE_OS_HW_WEIRD_INTERRUPTS
	cli();
	#endif

	PUSH(31);PUSH(30);PUSH(29);PUSH(28);PUSH(27);PUSH(26);PUSH(25);PUSH(24);
	PUSH(23);PUSH(22);PUSH(21);PUSH(20);PUSH(19);PUSH(18);PUSH(17);PUSH(16);
	PUSH(15);PUSH(14);PUSH(13);PUSH(12);PUSH(11);PUSH(10);PUSH( 9);PUSH( 8);
	PUSH( 7);PUSH( 6);PUSH( 5);PUSH( 4);PUSH( 3);PUSH( 2);PUSH( 1);PUSH( 0);
	asm volatile("in r0,__SREG__");
	PUSH(0);
	asm volatile("clr r1");
	
	_currentTask->_stackPointer = (uint8_t*)(SP);

	// Check stack
	blueOsAssert(  (SP >= (int)_currentTask->_stack)
				&& (SP < (int)(_currentTask->_stack + _currentTask->_stackSize)), BLUE_OS_PANIC_STACKPOINTER_OUTSIDE);

	blueOsAssert((uint8_t)_currentTask->_stack[0] == (uint8_t) BLUE_OS_STACK_MAGIC_NUMBER(_currentTaskId), BLUE_OS_PANIC_STACK_DAMAGED);

	blueOsAssert((uint8_t)(_currentTask->_stack[(_currentTask->_stackSize) - 1]) == (uint8_t)BLUE_OS_STACK_MAGIC_NUMBER(_currentTaskId), BLUE_OS_PANIC_STACK_DAMAGED);

	// Use the stack of main() for the scheduler
	SP = (uint16_t)(_idleTask._stackPointer);

	if (_isSoftInterrupt == (uint8_t) 0)
	{
		// Do this only, if it is a real time overflow!
		// Init timer
		TIMER_COUNTER += TIMER_COUNTER_INIT_VALUE;
		_milliseconds++;

		// Check delays
		#ifdef BLUE_OS_USE_DELAY
		if (_milliseconds >= _nextDelayTimeout)
		{
			blueOsDelaySchedule();
		}
		#endif

		
		#ifdef BLUE_OS_USE_USER_WATCHDOG
			// Check User Watchdogs
			if (_milliseconds >= _nextUserWatchdog)
			{
				blueOsUserWatchdogSchedule();
			}
		#endif //BLUE_OS_USE_USER_WATCHDOG
	}
	_isSoftInterrupt = 0;

	// Select next task
	#ifdef BLUE_OS_SCHEDULER_PRIO_RR
	_currentTaskId = blueOsSchedulePrioRR();
	#endif //BLUE_OS_SCHEDULER_PRIO_RR

	#ifdef BLUE_OS_SCHEDULER_OWN_IMPLEMENTATION
	_currentTaskId = blueOsScheduleOwnImplementation();
	#endif //BLUE_OS_SCHEDULER_OWN_IMPLEMENTATION

	_currentTask = _tasks[_currentTaskId];
	
	SP = (uint16_t)(_currentTask->_stackPointer);

	// Check stack
	blueOsAssert(  (SP >= (int)_currentTask->_stack)
				&& (SP < (int)(_currentTask->_stack + _currentTask->_stackSize)), BLUE_OS_PANIC_STACKPOINTER_OUTSIDE);

	blueOsAssert((uint8_t)_currentTask->_stack[0] == (uint8_t) BLUE_OS_STACK_MAGIC_NUMBER(_currentTaskId), BLUE_OS_PANIC_STACK_DAMAGED);

	blueOsAssert((uint8_t)(_currentTask->_stack[(_currentTask->_stackSize) - 1]) == (uint8_t)BLUE_OS_STACK_MAGIC_NUMBER(_currentTaskId), BLUE_OS_PANIC_STACK_DAMAGED);


	POP( 0);
	asm volatile("out __SREG__,r0");
	POP( 0);POP( 1);POP( 2);POP( 3);POP( 4);POP( 5);POP( 6);POP( 7);
	POP( 8);POP( 9);POP(10);POP(11);POP(12);POP(13);POP(14);POP(15);
	POP(16);POP(17);POP(18);POP(19);POP(20);POP(21);POP(22);POP(23);
	POP(24);POP(25);POP(26);POP(27);POP(28);POP(29);POP(30);POP(31);
	
	#ifdef BLUE_OS_HW_WEIRD_INTERRUPTS
	sei();
	#endif

	reti();
}


#ifdef BLUE_OS_USE_DELAY
void blueOsDelay(uint16_t delayMs)
{
	uint8_t irstate = blueOsEnterCriticalSection();

	uint32_t timeoutMs = _milliseconds + delayMs;

	_taskDelays[_currentTaskId] = timeoutMs;

	// Update the global timeout
	if (timeoutMs < _nextDelayTimeout)
		_nextDelayTimeout = timeoutMs;

	// Set task sleeping (not allowed for idle (main) task)
	if (_currentTaskId > 0)
	{
		_currentTask->_state = (uint8_t) BLUE_OS_STATE_WAITING;

		// Call the scheduler, because this task is sleeping now
		blueOsSoftTimerInterrupt();
	}

	blueOsLeaveCriticalSection(irstate);
}

	#ifdef BLUE_OS_USE_US_DELAY
void blueOsDelayUs(uint16_t delayUs)
{
	// Calculate timestamp
	static volatile uint16_t us;
	us = blueOsGetUs();
	delayUs = delayUs + us;

	static volatile uint16_t milliseconds;
	milliseconds = delayUs / 1000;
	_currentTask->_usDelay = delayUs - (milliseconds * 1000);

	if (milliseconds == 0)
	{
		// Directly register the us

		uint16_t timerValue = TIMER_COUNTER_INIT_VALUE + 
								((TIMER_TICKS_MS*delayUs)/1000UL);

		if (_currentUsDelayTask >= 0)
		{
			if (TIMER_COMPARE_VALUE > timerValue)
			{
				_currentUsDelayTask = _currentTask->_taskId;
				TIMER_COMPARE_VALUE = timerValue;
			}
		}
		else
		{
			_currentUsDelayTask = _currentTask->_taskId;
			TIMER_COMPARE_VALUE = timerValue;
		}
		TIMER_ENABLE_COMPARE;

		if (TIMER_COUNTER >= TIMER_COMPARE_VALUE)
		{
			TIMER_COMPARE_VALUE = TIMER_COUNTER;
		}
	}

	// Microseconds are registered, register the task in the
	// milliseconds schedule and sleep the task
	blueOsDelay(milliseconds);
}
	#endif //BLUE_OS_USE_US_DELAY
#endif //BLUE_OS_USE_DELAY
