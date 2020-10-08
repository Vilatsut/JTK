/*
 * tmp007.c
 *
 *  Created on: 28.9.2016
 *  Author: Teemu Leppanen / UBIComp / University of Oulu
 *
 *  Datakirja: http://www.ti.com/lit/ds/symlink/tmp007.pdf
 */

#include <xdc/runtime/System.h>
#include <string.h>
#include "Board.h"
#include "sensors/tmp007.h"

void tmp007_setup(I2C_Handle *i2c) {

	System_printf("TMP007: Config OK!\n");
    System_flush();
}

double tmp007_get_data(I2C_Handle *i2c) {

	double temperature = 0.0;
	I2C_Transaction i2cTransaction;

	/*
	// Viestipuskurit, esittele oikea koko (korvaa nn)
	uint8_t txBuffer[1];
    uint8_t rxBuffer[2];
	// Laitteen i2c-osoite
    i2cTransaction.slaveAddress = Board_TMP007_ADDR;
	// Laitteen rekisterin osoite
    txBuffer[0] = TMP007_REG_TEMP;
	// korvaa nn
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 2;
    */


	if (I2C_transfer(*i2c, &i2cTransaction)) {

    	// JTKJ: Tehtävä 2. Muunna rekisteriarvo lämpötilaksi (Celsius) Luentomateriaalin ja
		//       harjoitustehtäväkoodisi avulla
		//       Exercise 2. Convert the register value to temperature with your exercise code
		//       and sensor datasheet
		// temperature = ...
	    //uint16_t detached = rxBuffer >> 2;
	    //float temperature = 0.03125 * detached;
	} else {

		System_printf("TMP007: Data read failed!\n");
		System_flush();
	}

	return temperature;
}
