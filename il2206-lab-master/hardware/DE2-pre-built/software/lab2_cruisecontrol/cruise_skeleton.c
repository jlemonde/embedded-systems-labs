#include <stdio.h>
#include "system.h"
#include "includes.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"
#include "sys/alt_alarm.h"

#define DEBUG 1

#define HW_TIMER_PERIOD 100 /* 100ms */

/* Button Patterns */

#define GAS_PEDAL_FLAG      0x08
#define BRAKE_PEDAL_FLAG    0x04
#define CRUISE_CONTROL_FLAG 0x02
/* Switch Patterns */

#define TOP_GEAR_FLAG       0x00000002
#define ENGINE_FLAG         0x00000001

/* LED Patterns */

#define LED_RED_0 0x00000001 // Engine
#define LED_RED_1 0x00000002 // Top Gear
#define LED_GREEN_0 0x0001 // Cruise Control activated
#define LED_GREEN_2 0x0004 // Cruise Control Button
#define LED_GREEN_4 0x0010 // Brake Pedal
#define LED_GREEN_6 0x0040 // Gas Pedal
/*
 * Definition of Tasks
 */

#define TASK_STACKSIZE 2048


OS_STK StartTask_Stack[TASK_STACKSIZE];
OS_STK ControlTask_Stack[TASK_STACKSIZE];
OS_STK VehicleTask_Stack[TASK_STACKSIZE];
OS_STK ButtonsIO_Stack[TASK_STACKSIZE];
OS_STK SwitchIO_Stack[TASK_STACKSIZE];
OS_STK OverloadDetection_Stack[TASK_STACKSIZE];
OS_STK Extraload_Stack[TASK_STACKSIZE];


// Task Priorities

#define STARTTASK_PRIO     		5
#define VEHICLETASK_PRIO  		8
#define CONTROLTASK_PRIO	 	10
#define BUTTONSIO_PRIO			12
#define SWITCHIO_PRIO			14
#define EXTRALOAD_PRIO			16
#define OVERLOADDETECTION_PRIO	18


// Task Periods

#define CONTROL_PERIOD  300
#define VEHICLE_PERIOD  300
#define BS_PERIOD		100
#define HYPER_PERIOD	300
#define OVERLOAD_PERIOD	100

/*
 * Definition of Kernel Objects 
 */

// Mailboxes
OS_EVENT *Mbox_Throttle;
OS_EVENT *Mbox_Velocity;

// Semaphores

OS_EVENT *SemVehicle;
OS_EVENT *SemControl;
OS_EVENT *SemButtons;
OS_EVENT *SemSwitches;
OS_EVENT *SemOverload;
OS_EVENT *SemExtraload;

// Data of semaphores

OS_SEM_DATA *p_sem_data_vehicle;
OS_SEM_DATA *p_sem_data_control;
OS_SEM_DATA *p_sem_data_buttons;
OS_SEM_DATA *p_sem_data_switches;
OS_SEM_DATA *p_sem_data_overload;

// SW-Timer

OS_TMR *TimerVehicle;
OS_TMR *TimerControl;
OS_TMR *TimerButtons;
OS_TMR *TimerSwitches;
OS_TMR *TimerOverload;
OS_TMR *TimerWatchdog;
OS_TMR *TimerExtraload;

/*
 * Types
 */
enum active {
	on, off
};

enum active gas_pedal = off;
enum active brake_pedal = off;
enum active top_gear = off;
enum active engine = off;
enum active cruise_control = off;

/*
 * Global variables
 */
int delay; // Delay of HW-timer
INT16U led_green = 0; // Green LEDs
INT32U led_red = 0; // Red LEDs
INT8U ENGINE = 0;
INT8U TOP_GEAR = 0;
INT8U GAS_PEDAL = 0;
INT8U BRAKE_PEDAL = 0;
INT8U CRUISE_CONTROL = 0;

int buttons_pressed(void) {
	return ~IORD_ALTERA_AVALON_PIO_DATA(D2_PIO_KEYS4_BASE);
}

int switches_pressed(void) {
	return IORD_ALTERA_AVALON_PIO_DATA(DE2_PIO_TOGGLES18_BASE);
}

/*
 * ISR for HW Timer
 */
alt_u32 alarm_handler(void* context) {
	OSTmrSignal(); /* Signals a 'tick' to the SW timers */

	return delay;
}

/*
 * Callbacks resumes the suspended task
 */
void CallbackVehicle(void *ptmr,void *callback_arg) {
	OSSemPost(SemVehicle);
}
void CallbackControl(void *ptmr,void *callback_arg) {
	OSSemPost(SemControl);
}
void CallbackButtons(void *ptmr,void *callback_arg) {
	OSSemPost(SemButtons);
}
void CallbackSwitches(void *ptmr,void *callback_arg) {
	OSSemPost(SemSwitches);
}
void CallbackOverload(void *ptmr,void *callback_arg) {
	OSSemPost(SemOverload);
}

void CallbackExtraload(void *ptmr, void *callback_arg) {
	OSSemPost(SemExtraload);
}

void CallbackWatchdog(void *ptmr, void *callback_arg) {
	printf("CPU is at 100 percent usage!\n");
}


static int b2sLUT[] = { 0x40, //0
		0x79, //1
		0x24, //2
		0x30, //3
		0x19, //4
		0x12, //5
		0x02, //6
		0x78, //7
		0x00, //8
		0x18, //9
		0x3F, //-
		};

/*
 * convert int to seven segment display format
 */
int int2seven(int inval) {
	return b2sLUT[inval];
}

/*
 * output current velocity on the seven segement display
 */
void show_velocity_on_sevenseg(INT8S velocity) {
	int tmp = velocity;
	int out;
	INT8U out_high = 0;
	INT8U out_low = 0;
	INT8U out_sign = 0;

	if (velocity < 0) {
		out_sign = int2seven(10);
		tmp *= -1;
	} else {
		out_sign = int2seven(0);
	}

	out_high = int2seven(tmp / 10);
	out_low = int2seven(tmp - (tmp / 10) * 10);

	out = int2seven(0) << 21 | out_sign << 14 | out_high << 7 | out_low;
	IOWR_ALTERA_AVALON_PIO_DATA(DE2_PIO_HEX_LOW28_BASE, out);
}

/*
 * shows the target velocity on the seven segment display (HEX5, HEX4)
 * when the cruise control is activated (0 otherwise)
 */
void show_target_velocity(INT8U target_vel) {
	INT16U targetVelocityHex;
	INT8U targetDecimalHigh, targetDecimalLow;

	targetDecimalHigh = target_vel / 10;
	targetDecimalLow = target_vel - 10 * targetDecimalHigh;
	targetVelocityHex = (int2seven(targetDecimalHigh) << 7)
			+ (int2seven(targetDecimalLow));

	IOWR_ALTERA_AVALON_PIO_DATA(DE2_PIO_HEX_HIGH28_BASE, targetVelocityHex);
}

/*
 * indicates the position of the vehicle on the track with the four leftmost red LEDs
 * LEDR17: [0m, 400m)
 * LEDR16: [400m, 800m)
 * LEDR15: [800m, 1200m)
 * LEDR14: [1200m, 1600m)
 * LEDR13: [1600m, 2000m)
 * LEDR12: [2000m, 2400m]
 */
void show_position(INT16U position) {
	int positionLEDs = 1 << (17 - (position / 4000));
	int currentLEDs = IORD_ALTERA_AVALON_PIO_DATA(DE2_PIO_REDLED18_BASE);
	currentLEDs = currentLEDs & ~(0x3F << 12); 					//do not overwrite the other switches' LEDs
	IOWR_ALTERA_AVALON_PIO_DATA( DE2_PIO_REDLED18_BASE,
			currentLEDs | positionLEDs);
}

/*
 * The function 'adjust_position()' adjusts the position depending on the
 * acceleration and velocity.
 */
INT16U adjust_position(INT16U position, INT16S velocity, INT8S acceleration,
		INT16U time_interval) {
	INT16S new_position = position + velocity * time_interval / 1000
			+ acceleration / 2 * (time_interval / 1000)
					* (time_interval / 1000);

	if (new_position > 24000) {
		new_position -= 24000;
	} else if (new_position < 0) {
		new_position += 24000;
	}

	show_position(new_position);
	return new_position;
}

/*
 * The function 'adjust_velocity()' adjusts the velocity depending on the
 * acceleration.
 */
INT16S adjust_velocity(INT16S velocity, INT8S acceleration,
		enum active brake_pedal, INT16U time_interval) {
	INT16S new_velocity;
	INT8U brake_retardation = 200;

	if (brake_pedal == off)
		new_velocity = velocity
				+ (float) (acceleration * time_interval) / 1000.0;
	else {
		if (brake_retardation * time_interval / 1000 > velocity)
			new_velocity = 0;
		else
			new_velocity = velocity - brake_retardation * time_interval / 1000;
	}

	return new_velocity;
}

/*
 * The task 'VehicleTask' updates the current velocity of the vehicle
 */
void VehicleTask(void* pdata) {
	INT8U err;
	void* msg;
	INT8U* throttle;
	INT8S acceleration; /* Value between 40 and -20 (4.0 m/s^2 and -2.0 m/s^2) */
	INT8S retardation; /* Value between 20 and -10 (2.0 m/s^2 and -1.0 m/s^2) */
	INT16U position = 0; /* Value between 0 and 20000 (0.0 m and 2000.0 m)  */
	INT16S velocity = 0; /* Value between -200 and 700 (-20.0 m/s amd 70.0 m/s) */
	INT16S wind_factor; /* Value between -10 and 20 (2.0 m/s^2 and -1.0 m/s^2) */

	printf("Vehicle task created!\n");

	while (1) {
		err = OSMboxPost(Mbox_Velocity, (void *) &velocity);

		OSSemPend(SemVehicle,0,&err);

		/* Non-blocking read of mailbox:
		 - message in mailbox: update throttle
		 - no message:         use old throttle
		 */
		msg = OSMboxPend(Mbox_Throttle, 1, &err);
		if (err == OS_NO_ERR)
			throttle = (INT8U*) msg;

		/* Retardation : Factor of Terrain and Wind Resistance */
		if (velocity > 0)
			wind_factor = velocity * velocity / 10000 + 1;
		else
			wind_factor = (-1) * velocity * velocity / 10000 + 1;

		if (position < 4000)
			retardation = wind_factor; // even ground
		else if (position < 8000)
			retardation = wind_factor + 15; // traveling uphill
		else if (position < 12000)
			retardation = wind_factor + 25; // traveling steep uphill
		else if (position < 16000)
			retardation = wind_factor; // even ground
		else if (position < 20000)
			retardation = wind_factor - 10; //traveling steep downhill
		else
			retardation = wind_factor - 5; // traveling downhill

		acceleration = *throttle / 2 - retardation;
		position = adjust_position(position, velocity, acceleration, 300);
		velocity = adjust_velocity(velocity, acceleration, brake_pedal, 300);
		printf("Position: %dm\n", position / 10);
		printf("Velocity: %4.1fm/s\n", velocity / 10.0);
		printf("Throttle: %dV\n", *throttle / 10);
		show_velocity_on_sevenseg((INT8S) (velocity / 10));
	}
}

/*
 * The task 'ControlTask' is the main task of the application. It reacts
 * on sensors and generates responses.
 */

void ControlTask(void* pdata) {
	INT8U err;
	INT8U throttle = 0; /* Value between 0 and 80, which is interpreted as between 0.0V and 8.0V */
	void* msg;
	INT16S* current_velocity;
	INT8U target_velocity;
	INT16U tempo;

	printf("Control Task created!\n");

	while (1) {
		msg = OSMboxPend(Mbox_Velocity, 0, &err);
		current_velocity = (INT16S*) msg;

		if(!ENGINE && !(*current_velocity)) engine = off;
		else engine = on;
		if(engine == on) {
			if(TOP_GEAR) top_gear = on;
			else top_gear = off;

			if(BRAKE_PEDAL) brake_pedal = on;
			else brake_pedal = off;

			if(GAS_PEDAL) gas_pedal = on;
			else gas_pedal = off;

			if(gas_pedal == on) {
				if(throttle >= 75) throttle = 80;
				else throttle += 5;
			}
			if(brake_pedal == on) {
				if(throttle <= 5 || ((INT8U) *current_velocity) == 0) throttle = 0;
				else throttle -= 5;
			}

			if((*current_velocity) >= 200 && CRUISE_CONTROL) {
				if(cruise_control == off) target_velocity = (INT8U) (*current_velocity / 10);
				cruise_control = on;
				show_target_velocity(target_velocity);
			}

			if(gas_pedal == on || brake_pedal == on || top_gear == off) {
				cruise_control = off;
				tempo = IORD_ALTERA_AVALON_PIO_DATA(DE2_PIO_GREENLED9_BASE);
				IOWR_ALTERA_AVALON_PIO_DATA(DE2_PIO_GREENLED9_BASE,tempo & ~1);
				target_velocity = 0;
				show_target_velocity(target_velocity);
			}

			if(cruise_control == on) {

				printf("Target Velocity: %d\n",(int) (target_velocity));
				tempo = IORD_ALTERA_AVALON_PIO_DATA(DE2_PIO_GREENLED9_BASE);
				tempo = tempo & ~1;
				IOWR_ALTERA_AVALON_PIO_DATA(DE2_PIO_GREENLED9_BASE,tempo | 1);	// writes 1 to the "cruise control" led w/o overwriting the others

				int delta = (*current_velocity)-10* ((int)target_velocity);
				if(delta >= 20){
					if(throttle <= 10) throttle = 0;
					else throttle -= 10;
				}
				else if(delta >= 10){
					if(throttle <= 5) throttle = 0;
					else throttle -= 5;
				}
				else if(delta >= 4){
					if(throttle != 0) throttle--;
				}
				else if(delta <= -20){
					if(throttle >= 70) throttle = 80;
					else throttle += 10;
				}
				else if(delta <= -10){
					if(throttle >= 75) throttle = 80;
					else throttle += 5;
				}
				else if(delta <= -4){
					if(throttle != 80) throttle ++;
				}
			}
		}

		err = OSMboxPost(Mbox_Throttle, (void *) &throttle);

		OSSemPend(SemControl,0,&err);
	}
}

/*
 * The task 'SwitchIO' read the switches periodically and lights up the red leds
 */

void SwitchIO(void* pdata) {
	INT8U err;
	INT32U current_led;
	while(1) {
		led_red = 0;
		if(switches_pressed() & TOP_GEAR_FLAG) {
			TOP_GEAR = 1;
			led_red += LED_RED_1;
		}
		else {
			TOP_GEAR = 0;
		}
		if(switches_pressed() & ENGINE_FLAG) {
			ENGINE = 1;
			led_red += LED_RED_0;
		}
		else {
			ENGINE = 0;
		}
		current_led = IORD_ALTERA_AVALON_PIO_DATA(DE2_PIO_REDLED18_BASE);
		current_led = current_led & ~(0xfff);
		IOWR_ALTERA_AVALON_PIO_DATA(DE2_PIO_REDLED18_BASE,led_red | current_led);

		OSSemPend(SemSwitches,0,&err);
	}
}

/*
 * The task 'ButtonsIO' read the buttons periodically and lights up the green leds
 */

void ButtonsIO(void* pdata) {
	INT8U err;
	INT16U current_led;
	while(1) {
		led_green = 0;
		if(buttons_pressed() & GAS_PEDAL_FLAG) {
			GAS_PEDAL = 1;
			led_green += LED_GREEN_6;
		}
		else {
			GAS_PEDAL = 0;
		}
		if(buttons_pressed() & BRAKE_PEDAL_FLAG) {
			BRAKE_PEDAL = 1;
			led_green += LED_GREEN_4;
		}
		else {
			BRAKE_PEDAL = 0;
		}
		if(buttons_pressed() & CRUISE_CONTROL_FLAG) {
			CRUISE_CONTROL = 1;
			led_green += LED_GREEN_2;
		}
		else {
			CRUISE_CONTROL = 0;
		}
		current_led = IORD_ALTERA_AVALON_PIO_DATA(DE2_PIO_GREENLED9_BASE);
		current_led = current_led & 0x01;
		IOWR_ALTERA_AVALON_PIO_DATA(DE2_PIO_GREENLED9_BASE,led_green | current_led);

		OSSemPend(SemButtons,0,&err);
	}
}

void OverloadDetection(void* pdata) {
	INT8U err;
	while(1) {
		OSSemPend(SemOverload,0,&err);
		OSTmrStart(TimerWatchdog,&err);
	}
}

void Extraload(void* pdata) {
	INT32U current_led;
	INT8U err;
	unsigned int OSCPUUsage = 0;
	INT8U extra = 0;
	long int j;
	while(1) {
		extra = ((switches_pressed()>>4) & 0x3f);
		current_led = IORD_ALTERA_AVALON_PIO_DATA(DE2_PIO_REDLED18_BASE);
		current_led = current_led & 0x3fc0f;
		IOWR_ALTERA_AVALON_PIO_DATA(DE2_PIO_REDLED18_BASE, (extra<<4) | current_led);

		if (extra > 50) extra = 50;
		j = 0;
		do {
			OSCPUUsage = 100 - (int)((float)OSIdleCtr/((float)OSIdleCtrMax/100.));
			j++;
		} while((int) (2*extra-1) > (unsigned int)OSCPUUsage && j < 30);

		printf("CPU Usage : %u, there were %ld loops executed and extra equals %d. Moreover we have %d and %d.\n", OSCPUUsage, j, (int) extra, (unsigned) OSIdleCtr, (unsigned) OSIdleCtrMax);

		OSSemPend(SemExtraload,0,&err);
	}
}

/* 
 * The task 'StartTask' creates all other tasks kernel objects and
 * deletes itself afterwards.
 */
void StartTask(void* pdata) {
	INT8U err;
	void* context;

	static alt_alarm alarm; /* Is needed for timer ISR function */

	/* Base resolution for SW timer : HW_TIMER_PERIOD ms */
	delay = alt_ticks_per_second() * HW_TIMER_PERIOD / 1000;
	printf("delay in ticks %d\n", delay);

	/*
	 * Create Hardware Timer with a period of 'delay'
	 */
	if (alt_alarm_start(&alarm, delay, alarm_handler, context) < 0) {
		printf("No system clock available!n");
	}

	/*
	 * Create and start Software Timer
	 */
	TimerVehicle = OSTmrCreate(0, VEHICLE_PERIOD / HW_TIMER_PERIOD,
			OS_TMR_OPT_PERIODIC, CallbackVehicle, NULL, NULL, &err);
	if (err) {
		printf("Error occurred while creating soft timer!\n");
	}
	OSTmrStart(TimerVehicle, &err);

	TimerControl = OSTmrCreate(0, CONTROL_PERIOD / HW_TIMER_PERIOD,
			OS_TMR_OPT_PERIODIC, CallbackControl, NULL, NULL, &err);
	if (err) {
		printf("Error occurred while creating soft timer!\n");
	}
	OSTmrStart(TimerControl, &err);

	TimerButtons = OSTmrCreate(0, BS_PERIOD / HW_TIMER_PERIOD,
			OS_TMR_OPT_PERIODIC, CallbackButtons, NULL, NULL, &err);
	if (err) {
		printf("Error occurred while creating soft timer!\n");
	}
	OSTmrStart(TimerButtons, &err);

	TimerSwitches = OSTmrCreate(0, BS_PERIOD / HW_TIMER_PERIOD,
			OS_TMR_OPT_PERIODIC, CallbackSwitches, NULL, NULL, &err);
	if (err) {
		printf("Error occurred while creating soft timer!\n");
	}
	OSTmrStart(TimerSwitches, &err);

	TimerExtraload = OSTmrCreate(0, HYPER_PERIOD / HW_TIMER_PERIOD,
			OS_TMR_OPT_PERIODIC, CallbackExtraload, NULL, NULL, &err);
	if (err) {
		printf("Error occurred while creating soft timer!\n");
	}
	OSTmrStart(TimerExtraload, &err);

	TimerOverload = OSTmrCreate(0, OVERLOAD_PERIOD / HW_TIMER_PERIOD,
			OS_TMR_OPT_PERIODIC, CallbackOverload, NULL, NULL, &err);
	if (err) {
		printf("Error occurred while creating soft timer!\n");
	}
	OSTmrStart(TimerOverload, &err);

	TimerWatchdog = OSTmrCreate(0, HYPER_PERIOD / HW_TIMER_PERIOD,
			OS_TMR_OPT_PERIODIC, CallbackWatchdog, NULL, NULL, &err);
	if (err) {
		printf("Error occurred while creating soft timer!\n");
	}
	OSTmrStart(TimerWatchdog, &err);

	/*
	 * Creation of Kernel Objects
	 */

	// Mailboxes
	Mbox_Throttle = OSMboxCreate((void*) 0); /* Empty Mailbox - Throttle */
	Mbox_Velocity = OSMboxCreate((void*) 0); /* Empty Mailbox - Velocity */

	// Semaphores
	SemVehicle = OSSemCreate(0);
	SemControl = OSSemCreate(0);
	SemButtons = OSSemCreate(0);
	SemSwitches = OSSemCreate(0);
	SemOverload = OSSemCreate(0);
	SemExtraload = OSSemCreate(1);

	/*
	 * Create statistics task
	 */

	OSStatInit();

	printf("OSIdleCtrMax : %d\n", (unsigned int)OSIdleCtrMax);

	/*
	 * Creating Tasks in the system
	 */

	err = OSTaskCreateExt(
			ControlTask, // Pointer to task code
			NULL, // Pointer to argument that is
			// passed to task
			&ControlTask_Stack[TASK_STACKSIZE - 1], // Pointer to top
			// of task stack
			CONTROLTASK_PRIO, CONTROLTASK_PRIO, (void *) &ControlTask_Stack[0],
			TASK_STACKSIZE, (void *) 0, OS_TASK_OPT_STK_CHK);
	if(err != 0) printf("Problem creating task : Control\n");

	err = OSTaskCreateExt(
			VehicleTask, // Pointer to task code
			NULL, // Pointer to argument that is
			// passed to task
			&VehicleTask_Stack[TASK_STACKSIZE - 1], // Pointer to top
			// of task stack
			VEHICLETASK_PRIO, VEHICLETASK_PRIO, (void *) &VehicleTask_Stack[0],
			TASK_STACKSIZE, (void *) 0, OS_TASK_OPT_STK_CHK);
	if(err != 0) printf("Problem creating task : Vehicle\n");

	err = OSTaskCreateExt(
				ButtonsIO, // Pointer to task code
				NULL, // Pointer to argument that is
				// passed to task
				&ButtonsIO_Stack[TASK_STACKSIZE - 1], // Pointer to top
				// of task stack
				BUTTONSIO_PRIO, BUTTONSIO_PRIO, (void *) &ButtonsIO_Stack[0],
				TASK_STACKSIZE, (void *) 0, OS_TASK_OPT_STK_CHK);
	if(err != 0) printf("Problem creating task : ButtonsIO\n");

	err = OSTaskCreateExt(
				SwitchIO, // Pointer to task code
				NULL, // Pointer to argument that is
				// passed to task
				&SwitchIO_Stack[TASK_STACKSIZE - 1], // Pointer to top
				// of task stack
				SWITCHIO_PRIO, SWITCHIO_PRIO, (void *) &SwitchIO_Stack[0],
				TASK_STACKSIZE, (void *) 0, OS_TASK_OPT_STK_CHK);
	if(err != 0) printf("Problem creating task : SwitchIO\n");


	err = OSTaskCreateExt(
				Extraload, // Pointer to task code
				NULL, // Pointer to argument that is
				// passed to task
				&Extraload_Stack[TASK_STACKSIZE - 1], // Pointer to top
				// of task stack
				EXTRALOAD_PRIO, EXTRALOAD_PRIO, (void *) &Extraload_Stack[0],
				TASK_STACKSIZE, (void *) 0, OS_TASK_OPT_STK_CHK);
	if(err != 0) printf("Problem creating task : Extraload : error %d\n", (unsigned) err);

	err = OSTaskCreateExt(
				OverloadDetection, // Pointer to task code
				NULL, // Pointer to argument that is
				// passed to task
				&OverloadDetection_Stack[TASK_STACKSIZE - 1], // Pointer to top
				// of task stack
				OVERLOADDETECTION_PRIO, OVERLOADDETECTION_PRIO, (void *) &OverloadDetection_Stack[0],
				TASK_STACKSIZE, (void *) 0, OS_TASK_OPT_STK_CHK);
	if(err != 0) printf("Problem creating task : Overload : Erreur %d\n", (unsigned int) err);



	printf("All Tasks and Kernel Objects generated!\n");

	/* Task deletes itself */

	OSTaskDel(OS_PRIO_SELF);
}

/*
 *
 * The function 'main' creates only a single task 'StartTask' and starts
 * the OS. All other tasks are started from the task 'StartTask'.
 *
 */

int main(void) {

	printf("Lab: Cruise Control\n");

	OSTaskCreateExt(
			StartTask, // Pointer to task code
			NULL, // Pointer to argument that is
			// passed to task
			(void *) &StartTask_Stack[TASK_STACKSIZE - 1], // Pointer to top
			// of task stack
			STARTTASK_PRIO, STARTTASK_PRIO, (void *) &StartTask_Stack[0],
			TASK_STACKSIZE, (void *) 0,
			OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	OSStart();

	return 0;
}
