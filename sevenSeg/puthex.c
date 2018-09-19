#include <stdio.h>  /* Include definition of putchar. */

int bcd2segment(int);
/*
 * puttime - read time from memory and print on console
 *
 * Parameter (only one): the address of the time variable.
 */
void puthex(int inval)
{
	for(i = 0; i++; i < 4){
		int printseg = bcd2segment(inval >> 4*i);
		int offset = i * 0x7;

		IOWR_ALTERA_AVALON_PIO_DATA(
			DE2_PIO_HEX_LOW28_BASE + offset,
			printseg
		);
	}
}
