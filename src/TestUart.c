/*=============================================================================
 * All rights reserved.
 * License: bsd-3-clause (see LICENSE.txt)
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "TestUart.h"
#include "uart.h"
#include "sapi.h"
#include "stdlib.h"

#include "stdio.h"
#include "string.h"


 /*
   int val;
   char str[20];

   strcpy(str, "98993489");
   val = atoi(str);
   printf("String value = %s, Int value = %d\n", str, val);

   strcpy(str, "tutorialspoint.com");
   val = atoi(str);
   printf("String value = %s, Int value = %d\n", str, val);


   char str[30] = "2030300 This is test";
   char *ptr;
   long ret;

   ret = strtol(str, &ptr, 10);

 */

/*=====[Definition macros of private constants]==============================*/


/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

state_t state;  //Instacion la variable estado
uint8_t flag;

/*=====[Definitions of private global variables]=============================*/

char* itoa(int value, char* result, int base) {
   // check that the base if valid
   if (base < 2 || base > 36) { *result = '\0'; return result; }

   char* ptr = result, *ptr1 = result, tmp_char;
   int tmp_value;

   do {
      tmp_value = value;
      value /= base;
      *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
   } while ( value );

   // Apply negative sign
   if (tmp_value < 0) *ptr++ = '-';
   *ptr-- = '\0';
   while(ptr1 < ptr) {
      tmp_char = *ptr;
      *ptr--= *ptr1;
      *ptr1++ = tmp_char;
   }
   return result;
}

/*=====[Main function, program entry point after power on or reset]==========*/

int main( void )
{
   // ----- Setup -----------------------------------
	boardInit();
	state = standByState;			//Inicializo la variable state
	char text[] = "0";
	char date[50];
	char auxC[] = "cc.cc";

	//-- configuracion UART
	uartConfig( UART_USB, 115200 ); // Inicializar UART_USB a 115200 baudios
	uartConfig( UART_232, 115200 ); // Inicializar UART_232 a 115200 baudios

	//-- Configuracion String




	// ----- Repeat for ever -------------------------
	while( true ) {

		switch  (state) {

		case standByState:			// Estado principal

			printf("MENU PRINCIPAL\r\n");
			printf("(1) Medir Campo\r\n");
			printf("(2) ---\r\n");
			printf("(3) --- \r\n");
			scanf("%s",text);   // en el cutecom para scanf y printf usar CR/LF para interpretar el cierre del string
			printf("El valor ingresado: %s\r\n", text);
			state = workState;
			break;

		case workState:			// Estado de trabajo

			if (text[0] == '1'){
				text[0]= '0'; 		//clear text

				printf("ENCENDER SONDA... \r\n");
				uartTxWrite(UART_232, 'r');
				uartTxWrite(UART_232, '\r');
				uartTxWrite(UART_232, '\n');
				state = busyState;
				break;
				}

			if (text[0] == '2'){
				text[0]= '0'; 		//clear text
				state = standByState;
				break;
			}

			if (text[0] == '3'){
				text[0]= '0'; 		//clear text
				state = standByState;
				break;
			}

			state = standByState;
			text[0]= '0'; //clear text
			break;

		case busyState:				// Espera la respuesta de la sonda ":r" -SONDA ON-

			gpioToggle(LEDB);
			delay(100);
			date [0] = 0;			//clear text


			while (uartRxReady(UART_232)) { 		//Hay datos no leidos
				for (uint8_t i = 0; i < 50; i++){
					date[i]= uartRxRead(UART_232); //Almaceno Dato
					}
				}

			if ((date[0] == ':') && (date[1] == 'r')){  //Repuesta de sonda
				printf("ESTADO DE SONDA: ON\r\n");
				for (uint8_t i = 0; date[i] != '\0'; i++) {
					uartTxWrite(UART_USB, date[i]);
					}
				state = MeasureState;
				}
			break;

		case MeasureState:			// Estado de medicion

			printf("MEDIR CAMPO... \r\n");

			if (uartTxReady(UART_232)){  		//Hay espacio para escribir
				uartTxWrite(UART_232, 'D');         //Comando D5 -> Medir Campo
				uartTxWrite(UART_232, '5');         //
				uartTxWrite(UART_232, '\r');
				uartTxWrite(UART_232, '\n');
				}

			state = finishState;
			break;

		case finishState:

			gpioToggle(LED1);
			delay(100);
			date [0] = 0; 								//clear text

			char patron[] = "N";						//Patron fin del string.
			uint16_t patronSize = sizeof(patron);

			char buffer[] = ":Dxx.xxyy.yyzz.zzcc.ccN";  //Buffer recepcion de caracteres. 23caracteres + '/0' = longitud 24
			uint32_t longitud = sizeof(buffer);

			char* receiveBuffer = buffer;				// es lo mismo que: *receiveBuffer = &buffer[0]; asigno al puntero (* receiveBuffer) la direccion del primer valor del buffer.
			uint32_t* receiveBufferSize = &longitud;	//  asigno al puntero (* receiveBufferSize) la direccion del valor longitud.

			tick_t timeout = 5000;  					//tiempo en miliseg de espera. 5Seg.

			//Recibo y guardo caracteres hasta que llegue el caracter patron o finaliza por tiempo.
			if (receiveBytesUntilReceiveStringOrTimeoutBlocking(UART_232,patron,patronSize,receiveBuffer,receiveBufferSize,timeout)){
				gpioToggle(LED3);
				uartWriteString( UART_USB, buffer );
				uartTxWrite(UART_USB, '\r');
				uartTxWrite(UART_USB, '\n');
				delay(100);
				// falta verificar que llego :D o E1 .. E7
				for(uint32_t i=17 ; i<(longitud-1) ; i++ ){
					auxC[i-17]=buffer[i];
					uartTxWrite(UART_USB,auxC[i-17]);
					}
				}
			else{
				// tratar que paso cuando no llega el caracter patron N o se paso te tiempo. Ver los errores E1 E2..
				// en buffer siempre queda los caracteres recibidos se pueden analizar o descartar.
			}

			state = MeasureState;
			break;

		case errorState:

			break;

		default:
			state = standByState;
			break;

		}
	}

   // YOU NEVER REACH HERE, because this program runs directly or on a
   // microcontroller and is not called by any Operating System, as in the 

  //  if(  uartReadByte( UART_USB, &dato ) ){

       // Se reenvia el dato a la UART_USB realizando un eco de lo que llega
    //   uartWriteByte( UART_USB, dato );
    //}
   // case of a PC program.
   return 0;
}




