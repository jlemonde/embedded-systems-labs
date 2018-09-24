#include <stdio.h>  /* Include definition of putchar. */
#include "system.h"
#include "altera_avalon_pio_regs.h"

int bcd2sevenSeg(int intval);
/*
 * puttime - read time from memory and print on console
 *
 * Parameter (only one): the address of the time variable.
 */
int bcd2sevenSeg(int intval){
	// mask to keep only the four least significant bits
	intval = intval & 15;

	// initialise vector to be returned
	int r;

	// look up table
	switch(intval){
		case  0:	r = 0x40;		break;
		case  1:	r = 0x79;		break;
		case  2:	r = 0x24;		break;
		case  3:	r = 0x30;		break;
		case  4:	r = 0x19;		break;
		case  5:	r = 0x12;		break;
		case  6:	r = 0x02;		break;
		case  7:	r = 0x78;		break;
		case  8:	r = 0x00;		break;
		case  9:	r = 0x10;		break;
		case 10:	r = 0x08;		break;
		case 11:	r = 0x03;		break;
		case 12:	r = 0x27;		break;
		case 13:	r = 0x21;		break;
		case 14:	r = 0x06;		break;
		case 15:	r = 0x0E;		break;
	}

	return r & 127;
}


/*
void puthex(int intval) {
	int i;
	for(i = 0; i < 4; i++){
		int printseg = bcd2sevenSeg(intval >> 4*i);
		int offset = i * 0x7;

		IOWR_ALTERA_AVALON_PIO_DATA(
			DE2_PIO_HEX_HIGH28_BASE + offset,
			printseg
		);
	}
}
*/
void puthex(int intval) {
	int i, printseg = 0;
	for(i = 3; i >= 0; i--){
		printseg = printseg | (bcd2sevenSeg(intval >> 4*i) << 7*i);
	}
	IOWR_ALTERA_AVALON_PIO_DATA(DE2_PIO_HEX_LOW28_BASE, printseg);
}

