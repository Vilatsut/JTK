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
#include "buzzer.h"
#include "wireless/comm_lib.h"


// Task size in bytes
#define STACKSIZE 1024
Char sensorTaskStack[STACKSIZE];
Char dispTaskStack[STACKSIZE];
Char musicTaskStack[STACKSIZE];
Char commTaskStack[STACKSIZE];

enum state {UP = 0, DOWN, LEFT, RIGHT, READ, VICTORY, START, LOSER};	// All the states used in our state machine
enum state myState = START;									// Setting our initialization state to start

// GLOBAL EVENT VARIABLE
char event[20];

// GLOBAL TIME VARIABLE
char time[20];

// MPU GLOBAL VARIABLES
static PIN_Handle hMpuPin;
static PIN_State MpuPinState;
static PIN_Config MpuPinConfig[] = {
    Board_MPU_POWER  | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

static PIN_Handle buzzer;
static PIN_State buzzerState;

PIN_Config buzzerConfig[] = {
    Board_BUZZER | PIN_GPIO_OUTPUT_EN | PIN_PULLUP,
    PIN_TERMINATE
};

static PIN_Handle topButton;
static PIN_State topButtonState;

PIN_Config topButtonPress[] = {
   Board_BUTTON0 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
   PIN_TERMINATE
};


static PIN_Handle hButtons;
static PIN_State buttonState;

PIN_Config ButtonTableShutdown[] = {
   Board_BUTTON1 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
   PIN_TERMINATE
};
PIN_Config ButtonTableWakeUp[] = {
   Board_BUTTON1 | PIN_INPUT_EN | PIN_PULLUP | PINCC26XX_WAKEUP_NEGEDGE,
   PIN_TERMINATE
};


// MPUs own I2C interface
static const I2CCC26XX_I2CPinCfg i2cMPUCfg = {
    .pinSDA = Board_I2C0_SDA1,
    .pinSCL = Board_I2C0_SCL1
};

// COMMUNICATION TASK
Void communicateTask(UArg arg0, UArg arg1) {
	char msgBuffer[16];
	uint16_t senderAddr;
	char deviceId[4];
	char condition[10];

	char moves[4][10] = {
		"UP", "DOWN", "LEFT", "RIGHT"
	};

	int32_t result = StartReceive6LoWPAN();
	if (result != true) {
		System_abort("Wireless receive start failed");
	}

	while(1) {
		if ((myState == UP) || (myState == DOWN) || (myState == LEFT) || (myState == RIGHT)) {
			sprintf(msgBuffer, "event:%s", moves[myState]);
			Send6LoWPAN(IEEE80154_SERVER_ADDR, msgBuffer, strlen(msgBuffer));
			StartReceive6LoWPAN();
			Task_sleep(1500000 / Clock_tickPeriod);
		}
		if (GetRXFlag()) {
		    memset(msgBuffer,0,16);
		    Receive6LoWPAN(&senderAddr, msgBuffer, 16);
/*		    deviceId = strtok(msgBuffer, ",");
		    condition = strtok(NULL, ",");
		    if (deviceId == "170") {
		    	if (condition == "WIN") {
		    		myState = VICTORY;
		    	}
		    	else if (condition == "LOST GAME") {
		    		myState = LOSER;
		    	}
		    }
*/
		}
		Task_sleep(100000 / Clock_tickPeriod);
	}
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

		if(myState == READ){

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
				//event = "UP";
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

Void musicalTask(UArg arg0, UArg arg1) {
	while(1) {
		if (myState != START) {
		    buzzerOpen(buzzer);
			buzzerSetFrequency(294); // d
			Task_sleep(600000 / Clock_tickPeriod);
			buzzerSetFrequency(350); // f
			Task_sleep(400000 / Clock_tickPeriod);
			buzzerSetFrequency(294);
			Task_sleep(200000 / Clock_tickPeriod);
			buzzerSetFrequency(294);
			Task_sleep(100000 / Clock_tickPeriod);
			buzzerSetFrequency(392); // g
			Task_sleep(200000 / Clock_tickPeriod);
			buzzerSetFrequency(294);
			Task_sleep(200000 / Clock_tickPeriod);
			buzzerSetFrequency(262); // c
			Task_sleep(200000 / Clock_tickPeriod);
			buzzerSetFrequency(294);
			Task_sleep(600000 / Clock_tickPeriod);
			buzzerSetFrequency(440); // a
			Task_sleep(400000 / Clock_tickPeriod);
			buzzerSetFrequency(294);
			Task_sleep(200000 / Clock_tickPeriod);
			buzzerSetFrequency(294);
			Task_sleep(100000 / Clock_tickPeriod);
			buzzerSetFrequency(466); // a#
			Task_sleep(200000 / Clock_tickPeriod);
			buzzerSetFrequency(440);
			Task_sleep(200000 / Clock_tickPeriod);
			buzzerSetFrequency(350);
			Task_sleep(200000 / Clock_tickPeriod);
			buzzerSetFrequency(294);
			Task_sleep(200000 / Clock_tickPeriod);
			buzzerSetFrequency(440);
			Task_sleep(200000 / Clock_tickPeriod);
			buzzerSetFrequency(587); // 5d
			Task_sleep(200000 / Clock_tickPeriod);
			buzzerSetFrequency(294);
			Task_sleep(100000 / Clock_tickPeriod);
			buzzerSetFrequency(262);
			Task_sleep(200000 / Clock_tickPeriod);
			buzzerSetFrequency(262);
			Task_sleep(100000 / Clock_tickPeriod);
			buzzerSetFrequency(220); // 3a
			Task_sleep(200000 / Clock_tickPeriod);
			buzzerSetFrequency(330);
			Task_sleep(200000 / Clock_tickPeriod);
			buzzerSetFrequency(294);
			Task_sleep(200000 / Clock_tickPeriod);
			buzzerSetFrequency(4);
			Task_sleep(800000 / Clock_tickPeriod);
			buzzerSetFrequency(523);
			Task_sleep(200000 / Clock_tickPeriod);
			buzzerSetFrequency(4);
			Task_sleep(100000 / Clock_tickPeriod);
			buzzerSetFrequency(523);
			Task_sleep(200000 / Clock_tickPeriod);
		    buzzerClose();
			Task_sleep(2000000 / Clock_tickPeriod);
		}
		else {
			Task_sleep(100000 / Clock_tickPeriod);
		}
	}
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
				  if (myState == START) {
					  GrImageDraw(pContext, &otitImage, 0, 10);
					  GrFlush(pContext);
					  Display_print0(displayHandle, 0, 9, "Start ^");
				  }
				  else if (myState == READ) {
					  GrImageDraw(pContext, &blankImage, 0, 0);
					  Display_print0(displayHandle, 0, 10, "Stop ^");
				  }
				  else if (myState == UP){
					  GrImageDraw(pContext, &upImage, 0, 5);
					  GrFlush(pContext);
					  Display_print0(displayHandle, 0, 10, "Stop ^");
					  Task_sleep(1500000 / Clock_tickPeriod);
					  if (myState != START) {
						  myState = READ;
					  }
				  }
				  else if (myState == DOWN){
					  GrImageDraw(pContext, &downImage, 0, 0);
					  GrFlush(pContext);
					  Display_print0(displayHandle, 0, 10, "Stop ^");
					  Task_sleep(1500000 / Clock_tickPeriod);
					  if (myState != START) {
						  myState = READ;
					  }
				  }
				  else if (myState == LEFT){
					  GrImageDraw(pContext, &leftImage, 0, 0);
					  GrFlush(pContext);
					  Display_print0(displayHandle, 0, 10, "Stop ^");
					  Task_sleep(1500000 / Clock_tickPeriod);
					  if (myState != START) {
						  myState = READ;
					  }
				  }
				  else if (myState == RIGHT){
					  GrImageDraw(pContext, &rightImage, 0, 0);
					  GrFlush(pContext);
					  Display_print0(displayHandle, 0, 10, "Stop ^");
					  Task_sleep(1500000 / Clock_tickPeriod);
					  if (myState != START) {
						  myState = READ;
					  }
				  }
				  else if (myState == VICTORY){
					  int i;
					  for(i = 96; i > 9; i--){
						  GrImageDraw(pContext, &otitImage, 0, i);
						  Display_print0(displayHandle, 11, 1, "Victory Royale");
						  GrFlush(pContext);
						  Task_sleep(20000 / Clock_tickPeriod);
					  }
					  char screenTime[20];
					  sprintf(screenTime, "time: %s", time);
					  Display_print0(displayHandle, 10, 3, screenTime);
					  Task_sleep(3000000 / Clock_tickPeriod);
					  GrImageDraw(pContext, &blankImage, 0, 0);
					  GrFlush(pContext);
					  myState = START;
				  }
				  else if (myState == LOSER){
					  int n;
					  for(n = 11; n >= 0; n--){
						  Display_print0(displayHandle, n, 0, "LOSERLOSERLOSER!");
						  Task_sleep(100000 / Clock_tickPeriod);
					  }
					  Task_sleep(1000000 / Clock_tickPeriod);
					  myState == START;
				  }
				  else {
					  Task_sleep(100000 / Clock_tickPeriod);
				  }
			 }
		}
	}
}


// POWER BUTTON TASK
Void powerFxn(PIN_Handle handle, PIN_Id pinId) {

	// Display on
	Display_Params params;
	Display_Params_init(&params);
	params.lineClearMode = DISPLAY_CLEAR_BOTH;
	Display_Handle displayHandle = Display_open(Display_Type_LCD, &params);

   // Display off
    Display_clear(displayHandle);
    Display_close(displayHandle);
    Task_sleep(100000 / Clock_tickPeriod);

    PIN_close(hButtons);
    PINCC26XX_setWakeup(ButtonTableWakeUp);
    Power_shutdown(NULL,0);
}

// MENU BUTTON TASK
Void buttonFxn(PIN_Handle handle, PIN_Id pinId) {
	if (myState == START) {
		myState = READ;
	}
	else {
		myState = START;
	}
}

// HANDLER FOR THE TIMER FUNCTION
Void clkFxn(UArg arg0) {

	sprintf(time,"%04.0f", (double)Clock_getTicks() / 100000.0);

}

int main(void) {

	Task_Handle sensorTask, dispTask, musicTask, commTask;
	Task_Params sensorTaskParams, dispTaskParams, musicTaskParams, commTaskParams;

    Board_initGeneral();
    Board_initI2C();

    Init6LoWPAN();

    // OPEN BUZZER PIN
    buzzer = PIN_open(&buzzerState, buzzerConfig);
    if (buzzer == NULL){
    	System_abort("Buzzer pin open failed!");
    }
    //buzzerOpen(buzzer);

    // OPEN TOP BUTTON PIN
    topButton = PIN_open(&topButtonState, topButtonPress);
    if (topButton == NULL) {
    	System_abort("Top button pin open failed!");
    }
    if (PIN_registerIntCb(topButton, &buttonFxn) != 0) {
	   System_abort("Error registering top button callback");
	}

    // OPEN MPU POWER PIN
    hMpuPin = PIN_open(&MpuPinState, MpuPinConfig);
    if (hMpuPin == NULL) {
    	System_abort("Sensor pin open failed!");
    }

    // SENSOR TASK
    Task_Params_init(&sensorTaskParams);
    sensorTaskParams.stackSize = STACKSIZE;
    sensorTaskParams.stack = &sensorTaskStack;
    sensorTask = Task_create((Task_FuncPtr)sensorFxn, &sensorTaskParams, NULL);
    if (sensorTask == NULL) {
    	System_abort("sensorTask create failed!");
    }

    // DISPLAY TASK
    Task_Params_init(&dispTaskParams);
    dispTaskParams.stackSize = STACKSIZE;
    dispTaskParams.stack = &dispTaskStack;
    dispTask = Task_create(displayTask, &dispTaskParams, NULL);
    if (dispTask == NULL) {
        System_abort("dispTask create failed!");
    }

    // MUSIC TASK
    Task_Params_init(&musicTaskParams);
	musicTaskParams.stackSize = STACKSIZE;
	musicTaskParams.stack = &musicTaskStack;
	musicTask = Task_create(musicalTask, &musicTaskParams, NULL);
	if (musicTask == NULL) {
		System_abort("musicTask create failed!");
	}

	//COMMUNICATION TASK
    Task_Params_init(&commTaskParams);
	commTaskParams.stackSize = STACKSIZE;
	commTaskParams.stack = &commTaskStack;
	commTask = Task_create(communicateTask, &commTaskParams, NULL);
	if (commTask == NULL) {
		System_abort("commTask create failed!");
	}

    hButtons = PIN_open(&buttonState, ButtonTableShutdown);
    if(!hButtons) {
	   System_abort("Error initializing power button\n");
    }
    if (PIN_registerIntCb(hButtons, &powerFxn) != 0) {
	   System_abort("Error registering power button callback");
    }


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


    // Start BIOS
    BIOS_start();

    return (0);
}
