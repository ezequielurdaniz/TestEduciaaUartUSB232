/*
 * uart.h
 *
 *  Created on: 14 oct. 2020
 *      Author: ezequiel
 */

#ifndef PROGRAMS_TESTUART_INC_UART_H_
#define PROGRAMS_TESTUART_INC_UART_H_

/*==================[inclusions]=============================================*/
#include "sapi.h"     // <= sAPI header
#include <stdint.h>
#include <stddef.h>

/*==================[(Proptotipo) functions definition]==========================*/
void uartInit_c(void);

void EnviarDatoUart (uartMap_t uart,uint8_t dato);
int RecivirDatoUart (uartMap_t uart,uint8_t dato);

/*==================[external data definition]===============================*/

uint8_t dato  ; //= 0;
uint8_t flagdat; // flagdat ON//OFF






#endif /* PROGRAMS_TESTUART_INC_UART_H_ */
