/*
  Functions.c

  Ingo Sander, 2005-10-04
  Johan Wennlund, 2008-09-19
  George Ungureanu, 2018-08-27

*/

#include <stdio.h>
#include <stdlib.h>
#include "system.h"
#include <time.h>
#include "altera_avalon_performance_counter.h"
//#include "alt_types.h"

#define M 64

int matrix[M][M];

/* Initialize the matrix */

void initMatrix (int matrix[][M]);
int  sumMatrix  (int matrix[][M], int size);

alt_u32 ticks;
alt_u32 time_1;
alt_u32 time_2;
alt_u32 timer_overhead;

float microseconds(int ticks)
{
  return (float) 1000000 * (float) ticks / (float) alt_get_cpu_freq();
}
/*
void start_measurement()
{
  alt_timestamp_start();
  time_1 = alt_timestamp();
}


void stop_measurement()
{
  time_2 = alt_timestamp();
  ticks = time_2 - time_1;
}
*/

int main ()
{

  int a;

  printf("Processor Type: %s\n", NIOS2_CPU_IMPLEMENTATION);

  /* Check if timer available */
  //PERF_START_MEASURING(P_COUNTER_BASE);

  /* Print frequency and period */
  printf("CPU frequency: %3.1f MHz\n", (float)alt_get_cpu_freq()/1000000.0);
  printf("CPU period:    %f ms\n\n", 1000.0/(float)alt_get_cpu_freq());

  /* Calculate Timer Overhead */
  // Average of 10 measurements */
  int i;
  PERF_RESET(P_COUNTER_BASE);
  for (i = 0; i < 100; i++) {
	  PERF_BEGIN(P_COUNTER_BASE, 0); 
	  PERF_END(P_COUNTER_BASE, 0);
  }
  timer_overhead = (int)perf_get_section_time(P_COUNTER_BASE, 0) / 100;


  printf("Timer overhead in ticks: %d\n", (int) timer_overhead);
  printf("Timer overhead in ms:    %f\n\n",
	 1000.0 * (float)timer_overhead/(float)alt_get_cpu_freq());

  printf("Measuring sumMatrix...");



  int measures_in_total = 100;
  int measures[measures_in_total];
  float measures_mean = 0;
  int j;
  for (j=0; j<measures_in_total; j++){
      initMatrix(matrix);
      PERF_RESET(P_COUNTER_BASE);
	  PERF_BEGIN(P_COUNTER_BASE, 0);
      a = sumMatrix (matrix, M);
	  PERF_END(P_COUNTER_BASE, 0);
      ticks = (int)perf_get_section_time(P_COUNTER_BASE, 0) / measures_in_total;
      measures[j] = ticks - timer_overhead;
      measures_mean += measures[j];
      printf("Test %d : %5.2f us (%d ticks)\n", j, microseconds(measures[j]), (int) (measures[j]));
  }

  measures_mean /= measures_in_total;
  float mean_average_error = 0;
  for(j = 0; j < measures_in_total; j++){
	  mean_average_error += abs(measures_mean - measures[j]);
  }
  mean_average_error /= measures_in_total;

  printf("Mean : %f ticks : %f ms; MAE %f ticks : %f ms \n", (float) measures_mean, 1000*(float) measures_mean / (float) alt_get_cpu_freq(), (float)mean_average_error, 1000*(float) mean_average_error / (float) alt_get_cpu_freq());

  printf("Done!\n");



  return 0;
}

void initMatrix (int matrix[M][M]){
  int i, j;

  for (i = 0; i < M; i++) {
    for (j = 0; j < M; j++) {
      matrix[i][j] = i+j;
    }
  }
}

int sumMatrix (int matrix[M][M], int size)
{
  int i, j, Sum = 0;

  for (i = 0; i < size; i ++) {
    for (j = 0; j < size; j ++) {
      Sum += matrix[i][j];
    }
  }
  return Sum;
}
