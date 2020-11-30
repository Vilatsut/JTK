/*
 * hdc1000.c
 *
 *  Created on: 22.7.2016
 *  Author: Teemu Leppanen / UBIComp / University of Oulu
 *
 * 	Datasheet http://www.ti.com/lit/ds/symlink/hdc1000.pdf
 */

#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Task.h>

#include "Board.h"
#include "sensors/hdc1000.h"

void hdc1000_setup(I2C_Handle *i2c) {

    System_printf("HDC1000: Do not use this sensor!\n");
    System_flush();


}

void hdc1000_get_data(I2C_Handle *i2c, double *temp, double *hum) {

    System_printf("HDC1000: Do not use this sensor!\n");
    System_flush();


	*temp = -1.0;
	*hum = - 1.0;
}

