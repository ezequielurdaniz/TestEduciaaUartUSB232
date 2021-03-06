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

void EnviarDatoUart (uartMap_t uart,uint8_t dato){

	 uartWriteByte(uart,dato);

	}

int RecivirDatoUart (uartMap_t uart,uint8_t dato){

	 uartReadByte( UART_USB, &dato );


	return dato;
}
