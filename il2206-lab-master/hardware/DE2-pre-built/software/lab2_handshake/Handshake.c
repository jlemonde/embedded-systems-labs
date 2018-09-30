// File: Handshake.c

#include <stdio.h>
#include "includes.h"
#include <string.h>

#define DEBUG 0

/* Definition of Semaphores */
OS_EVENT* PrintSem = NULL;

/* Definition of Task Stacks */
/* Stack grows from HIGH to LOW memory */
#define   TASK_STACKSIZE       2048
OS_STK task0_stk[TASK_STACKSIZE];
OS_STK task1_stk[TASK_STACKSIZE];
OS_STK stat_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */
#define TASK0_PRIORITY      6  // highest priority
#define TASK1_PRIORITY      7
#define TASK_STAT_PRIORITY 12  // lowest priority

INT8U state0 = 0;
INT8U state1 = 1;


/* ----------------------------------------------- */


/* Called by StatisticTask */
void printStackSize(char* name, INT8U prio)
{
  INT8U err;
  OS_STK_DATA stk_data;

  err = OSTaskStkChk(prio, &stk_data);
  if (err == OS_NO_ERR) {
    if (DEBUG == 1)
      printf("%s (priority %d) - Used: %d; Free: %d\n",
	     name, prio, stk_data.OSUsed, stk_data.OSFree);
  }
  else
    {
      if (DEBUG == 1)
	printf("Stack Check Error!\n");
    }
}


/* Prints a message and sleeps for given time interval */
void task0(void* pdata) {

	INT8U err0;
	INT16U timeout = 0xffff;

	while (1) {
		OSSemPend(PrintSem, timeout, &err0);
		if(err0 == 0) {
			while(state1) {
				printf("Task 0 - State %d\n", state0);
				if(!state0) {
					state1 = !state1;
					break;
				}
				state0 = !state0;
			}
			OSSemPost(PrintSem);
			OSTimeDlyHMSM(0,0,0,1);
		}
	}
}


/* Prints a message and sleeps for given time interval */
void task1(void* pdata) {

	INT8U err1;
	INT16U timeout = 0xffff;

	while (1) {
		OSSemPend(PrintSem, timeout, &err1);
		if(err1 == 0) {
			while(!state0) {
				printf("Task 1 - State %d\n", state1);
				if(state1) {
					state0 = !state0;
					break;
				}
				state1 = !state1;
			}
			OSSemPost(PrintSem);
			OSTimeDlyHMSM(0,0,0,1);
		}
	}
}


/* Printing Statistics */
void statisticTask(void* pdata) {

	INT8U errs;
	INT16U timeout = 0xffff;

	while (1) {
		OSSemPend(PrintSem, timeout, &errs);
		if (errs == 0) {
			printStackSize("Task0", TASK0_PRIORITY);
			printStackSize("Task1", TASK1_PRIORITY);
			printStackSize("StatisticTask", TASK_STAT_PRIORITY);
			OSSemPost(PrintSem);
		}
	}
}


/* The main function creates two task and starts multi-tasking */
int main(void) {
	printf("Lab 2 - Handshake\n");

	OSInit();

	OSTaskCreateExt(task0, 							// Pointer to task code
			NULL, 									// Pointer to argument passed to task
			&task0_stk[TASK_STACKSIZE - 1], 		// Pointer to top of task stack
			TASK0_PRIORITY, 						// Desired Task priority
			TASK0_PRIORITY, 						// Task ID
			&task0_stk[0], 							// Pointer to bottom of task stack
			TASK_STACKSIZE, 						// Stacksize
			NULL, 									// Pointer to user supplied memory (not needed)
			OS_TASK_OPT_STK_CHK | 					// Stack Checking enabled
				OS_TASK_OPT_STK_CLR 				// Stack Cleared
			);

	OSTaskCreateExt(task1, 							// Pointer to task code
			NULL, 									// Pointer to argument passed to task
			&task1_stk[TASK_STACKSIZE - 1], 		// Pointer to top of task stack
			TASK1_PRIORITY, 						// Desired Task priority
			TASK1_PRIORITY, 						// Task ID
			&task1_stk[0], 							// Pointer to bottom of task stack
			TASK_STACKSIZE, 						// Stacksize
			NULL, 									// Pointer to user supplied memory (not needed)
			OS_TASK_OPT_STK_CHK | 					// Stack Checking enabled
				OS_TASK_OPT_STK_CLR 				// Stack Cleared
			);

	if (DEBUG == 1) {
		OSTaskCreateExt(statisticTask, 				// Pointer to task code
				NULL, 								// Pointer to argument passed to task
				&stat_stk[TASK_STACKSIZE - 1], 		// Pointer to top of task stack
				TASK_STAT_PRIORITY,	 				// Desired Task priority
				TASK_STAT_PRIORITY, 				// Task ID
				&stat_stk[0], 						// Pointer to bottom of task stack
				TASK_STACKSIZE, 					// Stacksize
				NULL,								// Pointer to user supplied memory (not needed)
				OS_TASK_OPT_STK_CHK | 				// Stack Checking enabled
					OS_TASK_OPT_STK_CLR 			// Stack Cleared
				);
	}

	PrintSem = OSSemCreate(1);						// Create Print related semaphore

	OSStart();
	return 0;
}
