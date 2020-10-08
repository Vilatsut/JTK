#include <stdio.h>

/* XDCtools files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/PIN.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/mw/display/Display.h>
#include <ti/mw/display/DisplayExt.h>
#include <ti/drivers/UART.h>

/* Board Header files */
#include "Board.h"

#include "wireless/comm_lib.h"
#include "sensors/opt3001.h"

/* Task */
#define STACKSIZE 2048
Char labTaskStack[STACKSIZE];
// Char commTaskStack[STACKSIZE];

// JTKJ: Tehtävä 1. Painonappien alustus ja muuttujat
// JTKJ: Exercise 1. Pin configuration and variables here

// JTKJ: Tehtävä 1.Painonapin keskeytyksen käsittelijä
//       Käsittelijässä vilkuta punaista lediä
// JTKJ: Exercise 1. Pin interrupt handler
//       Blink the red led of the device

/* Task Functions */
static PIN_Handle buttonHandle;
static PIN_State buttonState;

static PIN_Handle ledHandle;
static PIN_State ledState;

PIN_Config buttonConfig[] = {
   Board_BUTTON0  | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE, // Neljän vakion TAI-operaatio
   PIN_TERMINATE };

PIN_Config ledConfig[] = {
   Board_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
   PIN_TERMINATE
};

void buttonFxn(PIN_Handle handle, PIN_Id pinId)
{

      // Vaihdetaan led-pinnin tilaa negaatiolla
      PIN_setOutputValue( ledHandle, Board_LED1, !PIN_getOutputValue( Board_LED1 ) );
}

Void labTaskFxn(UArg arg0, UArg arg1) {

    I2C_Handle      i2c;
    I2C_Params      i2cParams;
    //I2C_Transaction i2cTransaction;
    Display_Handle	displayHandle;
    UART_Handle uart;
    UART_Params uartParams;
    Display_Params params;
    Display_Params_init(&params);
    params.lineClearMode = DISPLAY_CLEAR_BOTH;

    // JTKJ: Tehtävä 2. Avaa i2c-väylä taskin käyttöön
    // JTKJ: Exercise 2. Open the i2c bus0

    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2c = I2C_open(Board_I2C_TMP, &i2cParams);
    if (i2c == NULL) {
       System_abort("Error Initializing I2C\n");
    }
    // JTKJ: Tehtävä 2. Sensorin alustus kirjastofunktiolla ennen datan lukemista
    // JTKJ: Exercise 2. Setup the sensor here, before its use
    opt3001_setup(&i2c);

	// JTKJ: Tehtävä 3. Näytön alustus
    // JTKJ: Exercise 3. Setup the display here
    displayHandle = Display_open(Display_Type_LCD, &params);

    // JTKJ: Tehtävä 4. UARTin alustus
    // JTKJ: Exercise 4. Setup UART connection
    // Alustetaan sarjaliikenne halutusti
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_TEXT;
    uartParams.readDataMode = UART_DATA_TEXT;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.readMode=UART_MODE_BLOCKING;
    uartParams.baudRate = 9600; // nopeus 9600baud
    uartParams.dataLength = UART_LEN_8; // 8
    uartParams.parityType = UART_PAR_NONE; // n
    uartParams.stopBits = UART_STOP_ONE; // 1

    uart = UART_open(Board_UART0, &uartParams);
       if (uart == NULL) {
          System_abort("Error opening the UART");
       }

    while (1) {

        // JTKJ: Tehtävä 2. Lue sensorilta dataa ja tulosta se Debug-ikkunaan
        // JTKJ: Exercise 2. Read sensor data and print it to the Debug window
    	float lux = opt3001_get_data(&i2c);

    	char string[16];
    	// JTKJ: Tehtävä 3. Tulosta sensorin arvo merkkijonoon ja kirjoita se ruudulle
		// JTKJ: Exercise 3. Store the sensor value as char array and print it to the display
    	if (displayHandle) {
    		 sprintf(string,"%f",lux);
    		 Display_print0(displayHandle, 0, 0, string);
    	}
    	char strang[64];
    	// JTKJ: Tehtävä 4. Lähetä CSV-muotoinen merkkijono UARTilla
		// JTKJ: Exercise 4. Send CSV string with UART
    	sprintf(strang, "id:%x,light=%.02f\r\n",Board_OPT3001_ADDR ,lux);
    	UART_write(uart, strang, strlen(strang));
    	// Once per second
    	Task_sleep(1000000 / Clock_tickPeriod);
    	Display_clear(displayHandle);

    }
}

/* Communication Task */
/*
Void commTaskFxn(UArg arg0, UArg arg1) {

    // Radio to receive mode
	int32_t result = StartReceive6LoWPAN();
	if(result != true) {
		System_abort("Wireless receive mode failed");
	}

    while (1) {

        // If true, we have a message
    	if (GetRXFlag() == true) {

    		// Handle the received message..
        }

    	// Absolutely NO Task_sleep in this task!!
    }
}
*/

Int main(void) {

    // Task variables
	Task_Handle labTask;
	Task_Params labTaskParams;
	/*
	Task_Handle commTask;
	Task_Params commTaskParams;
	*/

    // Initialize board
    Board_initGeneral();

    // JTKJ: Tehtävä 2. i2c-cäylä käyttöön ohjelmassa
    // JTKJ: Exercise 2. Use i2c bus in program

    Board_initI2C();

    // JTKJ: Tehtävä 4. UART käyttöön ohjelmassa
    // JTKJ: Exercise 4. Use UART in program
    Board_initUART();

    // JTKJ: Tehtävä 1. Painonappi- ja ledipinnit käyttöön tässä
	// JTKJ: Exercise 1. Open and configure the button and led pins here

    // JTKJ: Tehtävä 1. Rekisteröi painonapille keskeytyksen käsittelijäfunktio
	// JTKJ: Exercise 1. Register the interrupt handler for the button
	buttonHandle = PIN_open(&buttonState, buttonConfig);
	if(!buttonHandle) {
		System_abort("Error initializing button pins\n");
	   }
    ledHandle = PIN_open(&ledState, ledConfig);
    if(!ledHandle) {
	   System_abort("Error initializing LED pins\n");
       }

    /* Task */
    Task_Params_init(&labTaskParams);
    labTaskParams.stackSize = STACKSIZE;
    labTaskParams.stack = &labTaskStack;
    labTaskParams.priority=2;

    labTask = Task_create(labTaskFxn, &labTaskParams, NULL);
    if (labTask == NULL) {
    	System_abort("Task create failed!");
    }

    if (PIN_registerIntCb(buttonHandle, &buttonFxn) != 0) {
	   System_abort("Error registering button callback function");
    }

    /* Communication Task */
	/*
    Init6LoWPAN(); // This function call before use!

    Task_Params_init(&commTaskParams);
    commTaskParams.stackSize = STACKSIZE;
    commTaskParams.stack = &commTaskStack;
    commTaskParams.priority=1;

    commTask = Task_create(commTaskFxn, &commTaskParams, NULL);
    if (commTask == NULL) {
    	System_abort("Task create failed!");
    }
	*/

    /* Sanity check */
    System_printf("Hello world!\n");
    System_flush();
    
    /* Start BIOS */
    BIOS_start();

    return (0);
}

