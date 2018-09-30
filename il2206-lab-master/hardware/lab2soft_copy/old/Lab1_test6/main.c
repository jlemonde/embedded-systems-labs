
#include <stdio.h>
#include "system.h"
#include "altera_avalon_pio_regs.h"

extern void puttime(int* timeloc);
extern void puthex(int time);
extern void tick(int* timeloc);
extern void delay (int millisec);
extern int hexasc(int invalue);

#define TRUE	1

int timeloc = 0x5957; // start value given in hexadecimal/BCD-code

int main() {
	int count = 0;
	printf("Hello from Nios II !! !! !!\n");
	while(TRUE){
		IOWR_ALTERA_AVALON_PIO_DATA(DE2_PIO_REDLED18_BASE, ++count);
		puttime(&timeloc);
		puthex(timeloc);
		delay(1000);
		tick(&timeloc);
	}

	return 0;
}
