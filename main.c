
#include "blue_os.h"
#include <avr/interrupt.h>


#define TASKA_STACKSIZE 128
BlueOsTCB taskATCB;
uint8_t taskAStack[TASKA_STACKSIZE];

#define TASKB_STACKSIZE 128
BlueOsTCB taskBTCB;
uint8_t taskBStack[TASKB_STACKSIZE];

#define TASKC_STACKSIZE 128
BlueOsTCB taskCTCB;
uint8_t taskCStack[TASKC_STACKSIZE];

#define TASKD_STACKSIZE 128
BlueOsTCB taskDTCB;
uint8_t taskDStack[TASKD_STACKSIZE];


void taskA()
{
	long number = 0;
	
	while (1)
	{
		blueOsDelay(250);
	}

}

BlueOsSignal taskCSignal;

void taskC()
{
	volatile int i;
	while (1)
	{
		blueOsWait(&taskCSignal);
		i++;
	}
}

void taskD()
{
	volatile int i;
	while (1)
	{
		blueOsWait(&taskCSignal);
		i++;
	}
}

void taskB()
{
	
	blueOsInitSignal(&taskCSignal);

	blueOsCreateTask(&taskCTCB, taskCStack, TASKC_STACKSIZE, 0, taskC,0);
	blueOsCreateTask(&taskDTCB, taskDStack, TASKD_STACKSIZE, 0, taskD,0);

	while (1)
	{
		#ifdef BLUE_OS_USE_DELAY
		blueOsDelay(10);
		#else
		blueOsYield();
		#endif //BLUE_OS_USE_DELAY

		//blueOsSignalSingle(&taskCSignal);
		blueOsSignalBroadcast(&taskCSignal);
	}
}



int main()
{	
	// After this function, the main task will be the idle task
	// Do not enable interrupts, the OS will do, when it is started
	blueOsInit();

	blueOsCreateTask(&taskATCB, taskAStack, TASKA_STACKSIZE, 1, taskA,0);
	blueOsCreateTask(&taskBTCB, taskBStack, TASKB_STACKSIZE, 1, taskB,0);

	// This function will enable interrupts
	// This function will never return
	// Stack of main function is trashed
	blueOsStart();


	// Program counter will NEVER be here
	return 0;
}


#ifdef BLUE_OS_IMPLEMENT_OWN_IDLE_TASK
void blueOsOwnIdleTask()
{	
	while (1)
	{
		blueOsYield();
	}
}
#endif

#ifdef BLUE_OS_IMPLEMENT_PANIC_CALLBACK

blueOsThread reboot = 0;

void blueOsKernelPanicHook(volatile uint8_t errorValue)
{
//	reboot();
}
#endif //BLUE_OS_IMPLEMENT_PANIC_CALLBACK

