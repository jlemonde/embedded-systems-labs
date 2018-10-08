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
	     name, prio,(int) stk_data.OSUsed,(int) stk_data.OSFree);
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
	INT16S* tempo_buffer;
	INT8U aberrant = 0;
	INT16U j = 0;					// number of iterations of measure_cswitch that aren't aberrant
	INT16U mean = 0;
	INT16U tempo = 0;

	tempo_buffer = OSMemGet(Mem, &err);
	*tempo_buffer = 1;

	while (1) {
		OSSemPend(SemT0S0, timeout, &err);
		if(!err) {
			OSMemPut(Mem,tempo_buffer);
			printf("Sending   : %d\n", *tempo_buffer);
		}

		OSSemPost(SemT1S0);


//		PERF_RESET(PERFORMANCE_COUNTER_BASE);
//		PERF_BEGIN(PERFORMANCE_COUNTER_BASE,0);

		OSSemPend(SemT0S1, timeout, &err); // Context Switch calculated here

//		PERF_END(PERFORMANCE_COUNTER_BASE,0);
//		tempo = (int) perf_get_section_time(PERFORMANCE_COUNTER_BASE, 0);
//		tempo -= global_tempo[0]+global_tempo[1]+global_tempo[2];
//		if(tempo > 2*mean && j > 0) {			// aberrant value is defined as a value that is bigger than 1.75 times the last mean
//			printf("Aberrant measured value\n");
//			aberrant++;
//			if(aberrant > 9) {					// if there are more than 9 aberrant values consecutively, the measurement is reset
//				aberrant = 0;
//				j = 0;
//				mean = 0;
//			}
//		}
//		else {
//			j++;
//			mean = (mean*(j-1) + tempo)/(j);
//			printf("Measure: %d; Measure in us: %d; Mean: %d; Mean in us: %d; Iterations: %d\n", (int) tempo,
//						(int) (1000000* (float) tempo / (float) alt_get_cpu_freq()), (int) mean,
//						(int) (1000000* (float) mean / (float) alt_get_cpu_freq()), (int) j);
//			if(aberrant > 0) aberrant--;
//		}


		if(!err) {
			OSMemPut(Mem,tempo_buffer);
			printf("Receiving : %d\n", *tempo_buffer);
			*tempo_buffer = -(*tempo_buffer) + 1;
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
	INT16S* tempo_buffer;

	while (1) {
		OSSemPend(SemT1S0, timeout, &err);
		if(!err) {
		tempo_buffer = OSMemGet(Mem,&err);
		}

		if(DEBUG)
			OSTimeDlyHMSM(0,0,0,4);

		OSSemPost(SemT1S1);

		OSSemPend(SemT1S1, timeout, &err);
		if(!err) {
		*tempo_buffer *= (-1);
		OSMemPut(Mem,tempo_buffer);
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

	SemT0S0 = OSSemCreate(1);
	SemT0S1 = OSSemCreate(0);
	SemT1S0 = OSSemCreate(0);
	SemT1S1 = OSSemCreate(0);

	OSStart();
	return 0;
}
