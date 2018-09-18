#include <stdio.h>
#include <string.h>
#include <sys/alt_cache.h>
#include "sys/alt_alarm.h"

#define TRANSFER_LENGTH 1048576
#define ITERATIONS 100

int do_dma( int * __restrict__ dest_ptr, int * __restrict__ source_ptr, int length )
{
  int i;

  for( i = 0; i < (length >> 2); i++ )
  {
    *dest_ptr++ = *source_ptr++;
  }
  return( 0 );
}

int main( void )
{
  int i;
  int *source_ptr, *dest_ptr;
  int start_time, finish_time, total_time;

  // Buffers we'll be transferring to and from.
  source_ptr = (int*)malloc(TRANSFER_LENGTH);
  dest_ptr = (int*)malloc(TRANSFER_LENGTH);

  // Fill the source buffer and erase the dest buffer
  for( i = 0; i < (TRANSFER_LENGTH / 4); i++ )
  {
    *(source_ptr + i) = i;
    *(dest_ptr + i) = 0x0;
  }

  printf("This simple program copies %d bytes of data from a source buffer to a destination buffer.\n", TRANSFER_LENGTH);
  printf("The program performs %d iterations of the copy operation, and calculates the time spent.\n\n", ITERATIONS);

  printf("Copy beginning\n");

  start_time = alt_nticks();

  for( i = 0; i < ITERATIONS; i++ )
  {
    do_dma( dest_ptr, source_ptr, TRANSFER_LENGTH );
  }

  finish_time = alt_nticks();

  if( memcmp( dest_ptr, source_ptr, TRANSFER_LENGTH ) )
  {
    printf("ERROR: Source and destination data do not match. Copy failed.\n");
  }
  else
  {
    printf("SUCCESS: Source and destination data match. Copy verified.\n");
  }

  free(dest_ptr);
  free(source_ptr);

  total_time = ((finish_time - start_time) * 1000) /
                alt_ticks_per_second();

  printf("Total time: %d ms\n", total_time);

  return 0;
}
