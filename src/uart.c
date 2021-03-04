/*
 * uart.c
 *
 *  Created on: 14 oct. 2020
 *      Author: ezequiel
 */

#include "sapi.h"     // <= sAPI header
#include "uart.h"

/*==================[external functions definition]==========================*/

void uartInit_c(void){

	// Inicializar UART_USB a 115200 baudios
	uartConfig( UART_USB, 115200 );
	// Inicializar UART_232 a 115200 baudios
	uartConfig( UART_232, 115200 );

	}

void enviarDato(uint8_t dato){
	// Envia el dato a la UART_USB
	uartWriteByte( UART_USB, dato );
	}

int RecibirDato(uint8_t dato){
	// Recibe un byte de la UART_USB guardo dato.
	if(uartReadByte( UART_USB, &dato )){
		flagdat=ON;
	}
	else{
		flagdat=OFF;
	}
	return flagdat;
	}

