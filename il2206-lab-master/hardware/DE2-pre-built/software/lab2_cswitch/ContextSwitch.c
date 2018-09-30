// File: Handshake.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "system.h"
#include "includes.h"
#include "altera_avalon_performance_counter.h"


#define DEBUG 0

/* Definition of Semaphores */
OS_EVENT* MemSem = NULL;

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

INT8U state0 = 0;				// (initial) state of task 0
INT8U state1 = 1;				// (initial) state of task 1
INT16U j = 0;					// number of iterations of measure_cswitch that aren't aberrant
int mean = 0;

OS_MEM* Mem = NULL;				// Address of the shared memory
INT16S MemBuffer[2];			// Buffer for the shared memory


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


/* Called by Task0 */
void measure_cswitch(int start_stop) {
	if(!start_stop) {
		PERF_END(PERFORMANCE_COUNTER_BASE,0);
		int tempo = (int)perf_get_section_time(PERFORMANCE_COUNTER_BASE, 0);
		if(tempo > 2*mean && j>1) {
			printf("Aberrant measured value\n");
			j--;
		}
		else mean = (mean*(j-1) + tempo)/j;

		printf("Measure:%d ; Mean in ticks:%d ; Mean in ms:%f ; Number of iterations:%d\n",tempo,
//				mean,(1000.0*(float)mean/(float)alt_get_cpu_freq()),j);
//		if(j==100) {
//			mean = 0;
//			j = 0;
//		}
	}
	if(start_stop) {
		PERF_RESET(PERFORMANCE_COUNTER_BASE);
		PERF_BEGIN(PERFORMANCE_COUNTER_BASE, 0);
		j++;
	}
}



/* Prints a message and sleeps for given time interval */
void task0(void* pdata) {

	INT8U err0;
	INT16U timeout = 0xffff, i = 1;
	INT16S* buffer;

	while (1) {
		OSSemPend(MemSem, timeout, &err0);
		if(err0 == 0) {
			while(state1) {
				if(!state0) {
					if(i==1) buffer = OSMemGet(Mem,&err0);

					printf("Sending : %d\n", i);
					*buffer = i++;
					OSMemPut(Mem,buffer);
					state1 = !state1;
					break;
				}
				buffer = OSMemGet(Mem,&err0);
				printf("Receiving : %d\n", *buffer);
				state0 = !state0;
			}
			OSSemPost(MemSem);
			measure_cswitch(1);
			OSTimeDlyHMSM(0,0,0,1);
		}
	}
}


/* Prints a message and sleeps for given time interval */
void task1(void* pdata) {

	INT8U err1;
	INT16U timeout = 0xffff;
	INT16S* buffer;

	while (1) {
		OSSemPend(MemSem, timeout, &err1);
		measure_cswitch(0);
		if(err1 == 0) {
			while(!state0) {
				if(state1) {
					OSMemPut(Mem,buffer);
					state0 = !state0;
					break;
				}
				buffer = OSMemGet(Mem,&err1);
				*buffer = -(*buffer);
				state1 = !state1;
			}
			OSSemPost(MemSem);
			OSTimeDlyHMSM(0,0,0,1);
		}
	}
}


/* Printing Statistics */
void statisticTask(void* pdata) {

	INT8U errs;
	INT16U timeout = 0xffff;

	while (1) {
		OSSemPend(MemSem, timeout, &errs);
		if (errs == 0) {
			printStackSize("Task0", TASK0_PRIORITY);
			printStackSize("Task1", TASK1_PRIORITY);
			printStackSize("StatisticTask", TASK_STAT_PRIORITY);
			OSSemPost(MemSem);
		}
	}
}


/* The main function creates two task and starts multi-tasking */
int main(void) {
	printf("Lab 2 - ContextSwitch\n");

	INT8U err;

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

	Mem = OSMemCreate(MemBuffer,2,
			sizeof(INT16U),&err);

	MemSem = OSSemCreate(1);						// Create Memory related semaphore

	OSStart();
	return 0;
}
