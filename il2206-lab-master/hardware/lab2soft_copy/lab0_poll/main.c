
#include <stdio.h>
#include "system.h"
#include "altera_avalon_pio_regs.h"

#define PROJECT_POLLING1

extern void puttime(int* timeloc);
extern void puthex(int time);
extern void tick(int* timeloc);
extern void delay (int millisec);
extern int hexasc(int invalue);

#define TRUE	1

void pollkey();

int timeloc = 0x5957; // start value given in hexadecimal/BCD-code
int is_counting = 1;
int buttons;
int buttons_previously;

int main() {
	int count = 0;
	printf("Hello from Nios II !! !! !!\n");
	while(TRUE){
		IOWR_ALTERA_AVALON_PIO_DATA(DE2_PIO_REDLED18_BASE, ++count);

        #ifdef PROJECT_POLLING1
		delay(1000);
		pollkey();
        #endif

		#ifdef PROJECT_POLLING2
		int i;
		for(i = 0; i < 1000; i++){
			delay(1);
			pollkey();
		}
		#endif

		if(is_counting){
			tick(&timeloc);
			puthex(timeloc);
			puttime(&timeloc);
		}
	}

	return 0;
}

void pollkey(){
	buttons_previously = buttons;
	buttons = IORD_ALTERA_AVALON_PIO_DATA(D2_PIO_KEYS4_BASE); // read values of the 4-bits buttons

	// the point in buttons_previously is to only capture the "descending slopes".
	if(~buttons & 0x1 && ! (~buttons_previously & 0x1))
		is_counting = 1;
	if(~buttons & 0x2 && ! (~buttons_previously & 0x2))
		is_counting = 0;
	if(~buttons & 0x4 && ! (~buttons_previously & 0x4)) {
		tick(&timeloc);
		puthex(timeloc);
		puttime(&timeloc);
	}
	if(~buttons & 0x8 && ! (~buttons_previously & 0x8)) {
		timeloc = 0x0000;
		puthex(timeloc);
		puttime(&timeloc);
	}
}
