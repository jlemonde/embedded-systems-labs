#include <stdio.h>  /* Include definition of putchar. */

int bcd2sevenSeg(int intval);
/*
 * puttime - read time from memory and print on console
 *
 * Parameter (only one): the address of the time variable.
 */
int bcd2sevenSeg(int intval){
	// keep only the four least significant bits
	intval = intval & 15;

	// initialise vector to be returned
	int r;

	// look up table
	switch(intval){
		case  0:	r = 0b01000000;		break;
		case  1:	r = 0b01111001;		break;
		case  2:	r = 0b00100100;		break;
		case  3:	r = 0b00110000;		break;
		case  4:	r = 0b00011001;		break;
		case  5:	r = 0b00010010;		break;
		case  6:	r = 0b00000010;		break;
		case  7:	r = 0b01111000;		break;
		case  8:	r = 0b00000000;		break;
		case  9:	r = 0b00010000;		break;
		case 10:	r = 0b00001000;		break;
		case 11:	r = 0b00000011;		break;
		case 12:	r = 0b00100111;		break;
		case 13:	r = 0b00100001;		break;
		case 14:	r = 0b00000110;		break;
		case 15:	r = 0b00001110;		break;
		default:    r = 0b01111111;     break;
	}

	return r;
}



void puthex(int intval)
{
	for(i = 0; i++; i < 4){
		int printseg = bcd2sevenSeg(intval >> 4*i);
		int offset = i * 0x7;

		IOWR_ALTERA_AVALON_PIO_DATA(
			DE2_PIO_HEX_LOW28_BASE + offset,
			printseg
		);
	}
}
