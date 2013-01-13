
/********************************************************************************
*Copyright (C) 2013 	Moritz Nagel 	mnagel@htwg-konstanz.de					*
*					Daniel Urbanietz 	daniel@d-urbanietz.de					*
*********************************************************************************
*FILE: blue_os_config.h															*
*																				*
*AUTHOR(S): M.Nagel, D.Urbanietz												*
*																				*
*DESCRIPTION:																	*
*This is the main config file for the BlueOS									*
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

#ifndef BLUE_OS_CONFIG_H
#define BLUE_OS_CONFIG_H

// Defines

/**
 * This constant configures the maximum count of usertasks,
 * that can be scheduled at the same time.
 */
#define BLUE_OS_MAX_TASKS 8

/**
 * This is the count of system tasks of the kernel,
 * do not modify, unless you implement deep kernel tasks.
 */
#define BLUE_OS_SYSTEM_TASKS 1

/**
 * This makro calculates the maximum count of tasks.
 */
#define BLUE_OS_TASK_COUNT (BLUE_OS_MAX_TASKS + BLUE_OS_SYSTEM_TASKS)

/**
 * This constant configures the stacksize in [Byte] of the
 * idle task.
 * @warning The idle tasks stack is also the system stack!
 */
#define BLUE_OS_SYSTEM_STACKSIZE 96

/**
 * This define configures the scheduler, which is used by the
 * OS. The following configurations are available:\n
 * - BLUE_OS_SCHEDULER_PRIO_RR: Uses a priority based round robin scheduling.\n
 * - BLUE_OS_SCHEDULER_OWN_IMPLEMENTATION: Uses a user implementation, of a scheduler.
 */
#define BLUE_OS_SCHEDULER_PRIO_RR
//#define BLUE_OS_SCHEDULER_OWN_IMPLEMENTATION

//#define BLUE_OS_SCHEDULE_TIME_MS 10 // Not used yet

/**
 * If BLUE_OS_DEBUG is defined, the debug mode inside the kernel
 * will be enabled. This mode adds assertions in critical functions.
 */
//#define BLUE_OS_DEBUG

/**
 * If BLUE_OS_IMPLEMENT_PANIC_CALLBACK is defined, the function:
 *	  void blueOsKernelPanicHook(volatile uint8_t errorValue)
 * must be implemented. This function will be called, if BLUE_OS_DEBUG
 * is enabled and a kernel panic was detected. errorValue is the
 * detected error.
 */
//#define BLUE_OS_IMPLEMENT_PANIC_CALLBACK

/**
 * If BLUE_OS_DEBUG_SHELL is enabled, a kernel panic debug message 
 * will be written to the selected shell.
 *
 * @warning BLUE_OS_USE_SHELL must be defined, to use this feature.
 */
#define BLUE_OS_DEBUG_SHELL

/**
 * If BLUE_OS_USE_SHELL is defined, one UART is used, to act as standard input/output.
 * These defines must be configured additionally:
 * 		BLUE_OS_USE_SHELL_UART
 *		BLUE_OS_USE_SHELL_BAUD_RATE
 *		BLUE_OS_SHELL_RXBUFFER
 *		BLUE_OS_SHELL_TXBUFFER
 */
#define BLUE_OS_USE_SHELL

/**
 * The define BLUE_OS_USE_SHELL_UART defines the selected UART for
 * the standard input/output.
 * 0: use UART0
 * ...
 * x: use UARTx
 */
#define BLUE_OS_USE_SHELL_UART 0 // 0: use UART0

/**
 * The define BLUE_OS_USE_SHELL_BAUD_RATE configures the baud rate of
 * the standard input/output. Don't forget, that this value is 32 bit,
 * so an 'L'´must be added after the value, e.g. 38400L for 38400 baud.
 * In case of xmegas you have to specify baud and scale manually (see Datasheet)
 */
#define BLUE_OS_USE_SHELL_BAUD_RATE 38400L
#define XMEGA_UART_BAUD 107
#define XMEGA_UART_SCALE 0b1011

/**
 * BLUE_OS_SHELL_RXBUFFER configures the size of the receive buffer of the
 * standard input.
 */
#define BLUE_OS_SHELL_RXBUFFER 32

/**
 * BLUE_OS_SHELL_TXBUFFER configures the size of the send buffer of the
 * standard output
 */
#define BLUE_OS_SHELL_TXBUFFER 32

/**
 * If BLUE_OS_USE_SHELL_VT100 is defined, the standard input/output is
 * used, to act as a VT100 terminal.
 *
 * @warning BLUE_OS_USE_SHELL must be defined, to use this feature
 */
//#define BLUE_OS_USE_SHELL_VT100
	#define BLUE_OS_SHELL_VT100_LISTENER

/**
 * If BLUE_OS_IMPLEMENT_OWN_IDLE_TASK is defined, an own idle
 * task can be implemented. In that case, the kernel will call
 * the function: void blueOsOwnIdleTask();
 */
//#define BLUE_OS_IMPLEMENT_OWN_IDLE_TASK

/**
 * If BLUE_OS_USE_SIGNAL is defined, signals will be enabled.
 */
#define BLUE_OS_USE_SIGNAL

/**
 * If BLUE_OS_USE_SEMA is defined, semaphores will be enabled
 */
//#define BLUE_OS_USE_SEMA

/**
 * If BLUE_OS_USE_EVENT is defined, the event-interface will be enabled
 */
//#define BLUE_OS_USE_EVENT
// Utils

/**
 * If BLUE_OS_US_TIMER this ist defined, the acces functions for
 * the us timer will be enabled.
 */
#define BLUE_OS_US_TIMER


/**
 * If BLUE_OS_USE_DELAY is defined, tasks can wait passively for
 * a variable time.
 */
#define BLUE_OS_USE_DELAY

/**
 * If BLUE_OS_USE_US_DELAY is defined, the BlueOS kernel can schedule
 * delays smaller than 1ms.
 */
//#define BLUE_OS_USE_US_DELAY


/**
 * If BLUE_OS_USE_USER_WATCHDOG is defined, watchdogs can be registered for every task.
 * With the function blueOsUserWatchdog(...), the timeout can be registered and updated.
 * If the task timed out, a callback function is called.
 */
//#define BLUE_OS_USE_USER_WATCHDOG

/**
 * If BLUE_OS_TASKS_NEVER_DEAD is defined, the return and kill functionality of tasks
 * will be disabled, so no task is allowed to terminate!
 * This can be used, to save program memory.
 *
 * @warning Use this only, if every task has an endless loop!
 */
//#define BLUE_OS_TASKS_NEVER_DEAD

/**
 * This define defines, which data type is used, to store the queue information.
 * The larger the type, the more data elements can be stored inside the queue.
 * E.g. uint8_t:  Queue can store 256 elements
 *		uint16_t: Queue can store 65536 elements
 *		...
 */
#define BLUE_OS_QUEUE_DATA_TYPE uint16_t

/**
 *  Memory Management to dynamically allocate and free memory 
 * 	
 */
//#define BLUE_OS_MEMORY

	/**
	 *	The memory in bytes which is dedicated to the memory management
	 *	
	 */
	#define BLUE_OS_MEM_SIZE 2048
	/**
	 *	The smallest block size in bytes
	 *	Only 256 blocks are supported
	 */
	#define BLUE_OS_MEMBLOCK_SIZE 16

	/**
	 *	Memory Managment Algorithms
	 *	
	 *	1. 	Buddy System
	 *		- BlockSize and MemorySize must be a power of two
	 *		- only powers of two can be allocated
	 *	-Standard Algorithms-
	 *	2. 	Rotate First Fit
	 *	3.  First Fit
	 *	4.  Best Fit
	 */

	#define BLUE_OS_MEM_BUDDY

	//#define BLUE_OS_MEM_STANDARD
		//#define BLUE_OS_MEM_RotateFirstFit
		#define BLUE_OS_MEM_FirstFit
		//#define BLUE_OS_MEM_BestFit

/**
 *  BLUE_OS_DYNAMIC_TASK provides a function to create a task dynamically. 
 * 	The required TCB and stack will be allocated by the MemoryManagement
 * 	
 *	int8_t blueOsCreateDynamicTask(uint16_t stackSize, uint8_t priority, blueOsThread _functionPointer, BlueOsTCB** handle);
 */
//#define BLUE_OS_DYNAMIC_TASK


#endif

