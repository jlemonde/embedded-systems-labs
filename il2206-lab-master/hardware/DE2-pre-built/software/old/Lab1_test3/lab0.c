#include <stdio.h>
#include "system.h"
#include "altera_avalon_pio_regs.h"

extern void puttime(int* timeloc);
extern void puthex(int time);
extern void tick(int* timeloc);
extern void delay (int millisec);
extern int hexasc(int invalue);

#define TRUE 1

int timeloc = 0x5957; /* startvalue given in hexadecimal/BCD-code */

int main ()
{
    int i = 0;
    while (TRUE)
    {
    	//delay(100);
    	long j=0;while(j++<100000);
    	IOWR_ALTERA_AVALON_PIO_DATA(DE2_PIO_REDLED18_BASE,++i);
        // puttime (&timeloc);
    }

    return 0;
}
