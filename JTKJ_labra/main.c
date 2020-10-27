/*
 * main.c
 *
 *  Created on: 20.10.2020
 *  Author: Ville Hokkinen and Juho Bruun
 *
 *	Main c file for the course assignment of the course Introduction to computer systems
 *
 */

// Standard headers
#include <stdio.h>
#include <string.h>

// XDCtools Headers
#include <xdc/std.h>
#include <xdc/runtime/System.h>

// BIOS Headers
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

// TI headers
#include <ti/drivers/I2C.h>
#include <ti/drivers/i2c/I2CCC26XX.h>
#include <ti/drivers/PIN.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

// Display headers
#include <splash_image.h>
#include <ti/mw/display/Display.h>
#include <ti/mw/display/DisplayExt.h>

// Board Headers
#include "Board.h"
#include "sensors/mpu9250.h"


// Task size in bytes
#define STACKSIZE 2048
Char sensorTaskStack[STACKSIZE];
Char dispTaskStack[STACKSIZE];


enum state {IDLE, UP, DOWN, LEFT, RIGHT, VICTORY}; 	// All the states used in our state machine
enum state myState = IDLE;							// Setting our initialization state to idle

// GLOBAL TIME VARIABLE
char time[4];

// MPU GLOBAL VARIABLES
static PIN_Handle hMpuPin;
static PIN_State MpuPinState;
static PIN_Config MpuPinConfig[] = {
    Board_MPU_POWER  | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

/*
static PIN_Handle powerHandle;
static PIN_State powerState;

PIN_Config powerConfig[] = {
   Board_BUTTON1 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
   PIN_TERMINATE
};
PIN_Config powerWakeConfig[] = {
   Board_BUTTON1 | PIN_INPUT_EN | PIN_PULLUP | PINCC26XX_WAKEUP_NEGEDGE,
   PIN_TERMINATE
};

PIN_Config buttonConfig[] = {
   Board_BUTTON1  | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
   PIN_TERMINATE
};
*/


// MPUs own I2C interface
static const I2CCC26XX_I2CPinCfg i2cMPUCfg = {
    .pinSDA = Board_I2C0_SDA1,
    .pinSCL = Board_I2C0_SCL1
};

Void moveTask() {

}

// SENSOR TASK
Void sensorFxn(UArg arg0, UArg arg1) {

	float ax, ay, az, gx, gy, gz;

    // MPU 9250 PARAMETERS
	I2C_Handle i2cMPU; // MPU9250 sensor interface
	I2C_Params i2cMPUParams;
    I2C_Params_init(&i2cMPUParams);
    i2cMPUParams.bitRate = I2C_400kHz;
    i2cMPUParams.custom = (uintptr_t)&i2cMPUCfg;

    // MPU OPEN I2C
    i2cMPU = I2C_open(Board_I2C, &i2cMPUParams);
    if (i2cMPU == NULL) {
        System_abort("Error Initializing I2CMPU\n");
    }

    // MPU POWER ON
    PIN_setOutputValue(hMpuPin,Board_MPU_POWER, Board_MPU_POWER_ON);

    // WAIT 100MS FOR THE SENSOR TO POWER UP
	Task_sleep(100000 / Clock_tickPeriod);
    System_printf("MPU9250: Power ON\n");
    System_flush();

    // MPU9250 SETUP
	System_printf("MPU9250: Setup and calibration...\n");
	System_flush();
	mpu9250_setup(&i2cMPU);
	System_printf("MPU9250: Setup and calibration OK\n");
	System_flush();

    // MPU CLOSE I2C
    I2C_close(i2cMPU);

	while (1) {

		if(myState == IDLE){

			// MPU OPEN I2C
			i2cMPU = I2C_open(Board_I2C, &i2cMPUParams);
			if (i2cMPU == NULL) {
				System_abort("Error Initializing I2CMPU\n");
			}

			// MPU ASK DATA
			mpu9250_get_data(&i2cMPU, &ax, &ay, &az, &gx, &gy, &gz);

		    // MPU CLOSE I2C
			I2C_close(i2cMPU);


			if(gx > 60) {
				myState = UP;
				Task_sleep(1500000 / Clock_tickPeriod);
			}
			else if(gx < -60) {
				myState = DOWN;
				Task_sleep(1500000 / Clock_tickPeriod);
			}
			else if(gy > 60) {
				myState = RIGHT;
				Task_sleep(1500000 / Clock_tickPeriod);
			}
			else if(gy < -60) {
				myState = LEFT;
				Task_sleep(1500000 / Clock_tickPeriod);
			}
			else {
				Task_sleep(100000 / Clock_tickPeriod);
			}
		}

		else {
	    	Task_sleep(100000 / Clock_tickPeriod);
	    }
    }

	// MPU9250 POWER OFF
	// Because of loop forever, code never goes here
    PIN_setOutputValue(hMpuPin,Board_MPU_POWER, Board_MPU_POWER_OFF);
}


Void displayTask(UArg arg0, UArg arg1) {

	// DISPLAY PARAMETERS
	Display_Params params;
	Display_Params_init(&params);
	params.lineClearMode = DISPLAY_CLEAR_BOTH;
	Display_Handle displayHandle = Display_open(Display_Type_LCD, &params);

    if (displayHandle) {

      tContext *pContext = DisplayExt_getGrlibContext(displayHandle);

		if (pContext){
			  while (1){
				  if (myState == UP){
					  GrImageDraw(pContext, &upImage, 0, 5);
					  GrFlush(pContext);
					  Task_sleep(1500000 / Clock_tickPeriod);
					  myState = IDLE;
					  GrClearDisplay(pContext);
				  }
				  else if (myState == DOWN){
					  GrImageDraw(pContext, &downImage, 0, 0);
					  GrFlush(pContext);
					  Task_sleep(1500000 / Clock_tickPeriod);
					  myState = IDLE;
					  GrClearDisplay(pContext);
				  }
				  else if (myState == LEFT){
					  GrImageDraw(pContext, &leftImage, 0, 0);
					  GrFlush(pContext);
					  Task_sleep(1500000 / Clock_tickPeriod);
					  myState = IDLE;
					  GrClearDisplay(pContext);
				  }
				  else if (myState == RIGHT){
					  GrImageDraw(pContext, &rightImage, 0, 0);
					  GrFlush(pContext);
					  Task_sleep(1500000 / Clock_tickPeriod);
					  myState = IDLE;
					  GrClearDisplay(pContext);
				  }
				  else if (myState == VICTORY){
					  int i;
					  for(i = 96; i > 16; i--){
						  GrImageDraw(pContext, &otitImage, 0, i);
						  GrFlush(pContext);
						  Task_sleep(25000 / Clock_tickPeriod);
					  }
					  for(i = 16; i > 4; i--){
						  GrClearDisplay(pContext);
						  GrImageDraw(pContext, &otitImage, 0, i);
						  GrFlush(pContext);
						  Task_sleep(25000 / Clock_tickPeriod);
					  }
					  Display_print0(displayHandle, 11, 1, "Victory Royale");
					  GrFlush(pContext);
					  Task_sleep(1000000 / Clock_tickPeriod);
				  }
				  else {
					  Task_sleep(100000 / Clock_tickPeriod);
				  }
			 }
		}
	}
}

/*
// Käsittelijäfunktio
Void powerFxn(PIN_Handle handle, PIN_Id pinId) {
	//Näyttö päälle
	Display_Params params;
	Display_Params_init(&params);
	params.lineClearMode = DISPLAY_CLEAR_BOTH;
	Display_Handle displayHandle = Display_open(Display_Type_LCD, &params);
   // Näyttö pois päältä
    Display_clear(displayHandle);
    Display_close(displayHandle);
    Task_sleep(100000 / Clock_tickPeriod);

   // Taikamenot
    PIN_close(powerHandle);
    PINCC26XX_setWakeup(powerWakeConfig);
    Power_shutdown(NULL,0);
}
*/
/*
Void clkFxn(UArg arg0) {

	// HANDLER FOR THE TIMER FUNCTION
	sprintf(time,"%04.0f", (double)Clock_getTicks() / 100000.0);
    Task_sleep(100000 / Clock_tickPeriod);
}
*/
int main(void) {

	Task_Handle sensorTask, dispTask;
	Task_Params sensorTaskParams, dispTaskParams;

    Board_initGeneral();
    Board_initI2C();

    // OPEN MPU POWER PIN
    hMpuPin = PIN_open(&MpuPinState, MpuPinConfig);
    if (hMpuPin == NULL) {
    	System_abort("Pin open failed!");
    }

    Task_Params_init(&sensorTaskParams);
    sensorTaskParams.stackSize = STACKSIZE;
    sensorTaskParams.stack = &sensorTaskStack;
    sensorTask = Task_create((Task_FuncPtr)sensorFxn, &sensorTaskParams, NULL);
    if (sensorTask == NULL) {
    	System_abort("Task create failed!");
    }

    Task_Params_init(&dispTaskParams);
    dispTaskParams.stackSize = STACKSIZE;
    dispTaskParams.stack = &dispTaskStack;
    dispTask = Task_create(displayTask, &dispTaskParams, NULL);
    if (dispTask == NULL) {
        System_abort("Task create failed!");
    }
/*
    powerHandle = PIN_open(&powerState, powerConfig);
       if(!powerHandle) {
          System_abort("Error initializing power button\n");
       }
       if (PIN_registerIntCb(powerHandle, &powerFxn) != 0) {
          System_abort("Error registering power button callback");
       }
*/
/*
    // RTOS' clock variables
    Clock_Handle clkHandle;
    Clock_Params clkParams;

    // Clock initialization
    Clock_Params_init(&clkParams);
    clkParams.period = 1000000 / Clock_tickPeriod;
    clkParams.startFlag = TRUE;

    // Introducing the clock in the program
    clkHandle = Clock_create((Clock_FuncPtr)clkFxn, 1000000 / Clock_tickPeriod, &clkParams, NULL);
       if (clkHandle == NULL) {
          System_abort("Clock create failed");
       }
*/
    // Start BIOS
    System_printf("Hello world!\n");
    System_flush();
    BIOS_start();

    return (0);
}
