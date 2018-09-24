
#include <stdio.h>
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"
#include "alt_types.h"

extern void puttime(int* timeloc);
extern void puthex(int time);
extern void tick(int* timeloc);
extern void delay (int millisec);
extern int hexasc(int invalue);

#define TRUE	1



static void init_button_pio();

static void key_InterruptHandler(void* context);



int timeloc = 0x5957; // start value given in hexadecimal/BCD-code
int is_counting = 1;
volatile int edge_capture;



int main() {
	int count = 0;
	printf("Hello from Nios II !! !! !!\n");

	init_button_pio();

	while(TRUE){
		IOWR_ALTERA_AVALON_PIO_DATA(DE2_PIO_REDLED18_BASE, ++count);

		delay(1000);

		if(is_counting){
			tick(&timeloc);
			puttime(&timeloc);
			puthex(timeloc);
		}
	}

	return 0;
}



static void init_button_pio()
	{
	/* Recast the edge_capture pointer to match the
	alt_irq_register() function prototype. */
	void* edge_capture_ptr = (void*) &edge_capture;

	/* Enable all 4 button interrupts. */
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(D2_PIO_KEYS4_BASE, 0xf);

	/* Reset the edge capture register. */
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(D2_PIO_KEYS4_BASE, 0x0);

	/* Register the ISR. */
	#ifdef ALT_ENHANCED_INTERRUPT_API_PRESENT
		alt_ic_isr_register(D2_PIO_KEYS4_IRQ_INTERRUPT_CONTROLLER_ID,
							D2_PIO_KEYS4_IRQ,
							key_InterruptHandler,
							edge_capture_ptr, 0x0);
	#else
		alt_irq_register(D2_PIO_KEYS4_IRQ,
						 edge_capture_ptr,
						 key_InterruptHandler);
	#endif
}



static void key_InterruptHandler(void* context)
{
	/* Cast context to edge_capture's type. It is important that this
	be declared volatile to avoid unwanted compiler optimization. */
	volatile int* edge_capture_ptr = (volatile int*) context;


	/* Read the edge capture register on the button PIO.
	* Store value.
	*/
	*edge_capture_ptr =
	IORD_ALTERA_AVALON_PIO_EDGE_CAP(D2_PIO_KEYS4_BASE);


	printf(" hello isr %d\n", *edge_capture_ptr);

	/* Write to the edge capture register to reset it. */
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(D2_PIO_KEYS4_BASE, 0);

	// Definition of  ISR

	if(*edge_capture_ptr & 0x1)
		is_counting = !is_counting;

	if(*edge_capture_ptr & 0x2){
		tick(&timeloc);
		puttime(&timeloc);
		puthex(timeloc);
	}

	if(*edge_capture_ptr & 0x4){
		timeloc = 0x0000;
		puttime(&timeloc);
		puthex(timeloc);
	}

	if(*edge_capture_ptr & 0x8){
		timeloc = 0x5957;
		puttime(&timeloc);
		puthex(timeloc);
	}

	/* Read the PIO to delay ISR exit. This is done to prevent a
	spurious interrupt in systems with high processor -> pio
	latency and fast interrupts. */
	IORD_ALTERA_AVALON_PIO_EDGE_CAP(D2_PIO_KEYS4_BASE);
}
