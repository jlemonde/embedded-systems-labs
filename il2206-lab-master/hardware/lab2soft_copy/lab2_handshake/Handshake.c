// File: Handshake.c

#include <stdio.h>
#include "includes.h"
#include <string.h>

#define DEBUG 0

/* Definition of Semaphores */
OS_EVENT* SemT0S0;
OS_EVENT* SemT0S1;
OS_EVENT* SemT1S0;
OS_EVENT* SemT1S1;

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
	     name, prio,(int)  stk_data.OSUsed,(int) stk_data.OSFree);
  }
  else
    {
      if (DEBUG == 1)
	printf("Stack Check Error!\n");
    }
}


/* Prints a message and sleeps for given time interval */
void task0(void* pdata) {

	INT8U err;
	INT16U timeout = 0;
	INT8U state = 0;

	while (1) {
		OSSemPend(SemT0S0, timeout, &err);
		if(!err) {
			printf("Task 0 - State %d\n", state);
			state = !state;
		}

		OSSemPost(SemT1S0);

		OSSemPend(SemT0S1, timeout, &err);
		if(!err) {
			printf("Task 0 - State %d\n", state);
			state = !state;
		}

		OSSemPost(SemT0S0);

		if(DEBUG)
			OSTimeDlyHMSM(0,0,0,4);
	}
}


/* Prints a message and sleeps for given time interval */
void task1(void* pdata) {

	INT8U err;
	INT16U timeout = 0;
	INT8U state = 0;

	while (1) {
		OSSemPend(SemT1S0, timeout, &err);
		if(!err) {
			printf("Task 1 - State %d\n", state);
			state = !state;
		}

		if(DEBUG)
			OSTimeDlyHMSM(0,0,0,4);

		OSSemPost(SemT1S1);

		OSSemPend(SemT1S1, timeout, &err);
		if(!err) {
			printf("Task 1 - State %d\n", state);
			state = !state;
		}

		OSSemPost(SemT0S1);
	}
}


/* Printing Statistics */
void statisticTask(void* pdata) {

	INT8U err;
	INT16U timeout = 0;

	while (1) {
		OSSemPend(SemT0S0, timeout, &err);
		if (!err) {
			printStackSize("Task0", TASK0_PRIORITY);
			printStackSize("Task1", TASK1_PRIORITY);
			printStackSize("StatisticTask", TASK_STAT_PRIORITY);
			OSSemPost(SemT0S0);
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

	SemT0S0 = OSSemCreate(1);
	SemT0S1 = OSSemCreate(0);
	SemT1S0 = OSSemCreate(0);
	SemT1S1 = OSSemCreate(0);

	OSStart();
	return 0;
}
