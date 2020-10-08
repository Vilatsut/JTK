/*
 * opt3001.c
 *
 *  Created on: 22.7.2016
 *  Author: Teemu Leppanen / UBIComp / University of Oulu
 *
 *  Datakirja: http://www.ti.com/lit/ds/symlink/opt3001.pdf
 */

#include <string.h>
#include <math.h>
#include <stdio.h>

#include <xdc/runtime/System.h>

#include "Board.h"
#include "sensors/opt3001.h"

void opt3001_setup(I2C_Handle *i2c) {

	I2C_Transaction i2cTransaction;
	char itxBuffer[3];

    i2cTransaction.slaveAddress = Board_OPT3001_ADDR;
    itxBuffer[0] = OPT3001_REG_CONFIG;
    itxBuffer[1] = 0xCE; // continuous mode s.22
    itxBuffer[2] = 0x02;
    i2cTransaction.writeBuf = itxBuffer;
    i2cTransaction.writeCount = 3;
    i2cTransaction.readBuf = NULL;
    i2cTransaction.readCount = 0;

    if (I2C_transfer(*i2c, &i2cTransaction)) {

        System_printf("OPT3001: Config write ok\n");
    } else {
        System_printf("OPT3001: Config write failed!\n");
    }
    System_flush();

}

uint16_t opt3001_get_status(I2C_Handle *i2c) {

	uint16_t e=0;
	I2C_Transaction i2cTransaction;
	char itxBuffer[1];
	char irxBuffer[2];

	/* Read sensor state */
	i2cTransaction.slaveAddress = Board_OPT3001_ADDR;
	itxBuffer[0] = OPT3001_REG_CONFIG;
	i2cTransaction.writeBuf = itxBuffer;
	i2cTransaction.writeCount = 1;
	i2cTransaction.readBuf = irxBuffer;
	i2cTransaction.readCount = 2;

	if (I2C_transfer(*i2c, &i2cTransaction)) {

		e = (irxBuffer[0] << 8) | irxBuffer[1];
	} else {

		e = 0;
	}
	return e;
}

double opt3001_get_data(I2C_Handle *i2c) {

	double lux = -1.0;
	I2C_Transaction i2cTransaction;

	if (opt3001_get_status(i2c) & OPT3001_DATA_READY) {

		// JTKJ: Tehtävä 2. Kommentit pois ja täytetään i2c-viestirakenne luentomateriaalin avulla..
		// JTKJ: Exercise 2. Uncomment and fill in the data structure below with correct values..

		// Viestipuskurit, esittele oikea koko (korvaa nn)
		uint8_t txBuffer[1];
		uint8_t rxBuffer[2];
		// Laitteen i2c-osoite
		i2cTransaction.slaveAddress = Board_OPT3001_ADDR;
		// Laitteen rekisterin osoite
		txBuffer[0] = OPT3001_REG_RESULT;
		// korvaa nn
		i2cTransaction.writeBuf = txBuffer;
		i2cTransaction.writeCount = 1;
		i2cTransaction.readBuf = rxBuffer;
		i2cTransaction.readCount = 2;


		if (I2C_transfer(*i2c, &i2cTransaction)) {

			// JTKJ: 1. Muunna mittausarvo 16-bit kokonaisluvuksi viestipuskurista
			//          (rxBuffer)
			//          Convert the register values (in rxBuffer) into 16 bit integer
			//double lux;
			//double exponent = 0b1111000000000000;
			//uint16_t fracresult = 0b0000111111111111;

			//int lux = 0.01* pow(2, (txBuffer & exponent) >> 12) * (txBuffer & fracresult);

		    uint16_t maski = 0b1111000000000000;
		    uint16_t recieved = (rxBuffer[0] << 8) | rxBuffer[1];
		    uint16_t E_rekisteri = recieved >> 12;
		    uint16_t R_rekisteri = recieved & ~(maski);
		    float lux = 0.01 * pow(2, E_rekisteri) * R_rekisteri;
		    char merkkijono[16];
			sprintf(merkkijono,"%f\n",lux);
	        System_printf(merkkijono);
	        System_flush();
			// JTKJ: 2. Laske mittausarvo luxeina luentomateriaalin (datakirjan)
			//          harjoitustehtäväsi kaavalla
			//          By using the equation in datasheet or your exercise
			//          find the value in lux
            // lux = ...
			return lux;
		} else {

			System_printf("OPT3001: Data read failed!\n");
			System_flush();
		}
	} else {
		System_printf("OPT3001: Data not ready!\n");
		System_flush();
	}


}
