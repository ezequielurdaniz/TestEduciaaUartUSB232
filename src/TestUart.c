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


/*=====[Definition macros of private constants]==============================*/


/*=====[Definitions of extern global variables]==============================*/


/*=====[Definitions of public global variables]==============================*/


/*=====[Definitions of private global variables]=============================*/

char* itoa(int value, char* result, int base); // Funcion int a char

void clearbuffer(char* buffer, uint32_t longitud); // Funcion Clear buffer

/*=====[Main function, program entry point after power on or reset]==========*/

int main( void )
{
	 // ----- Variables -----------------------------------
	state_t state = Base;						//Estado inicial de la maquina.

	// ----- Variables -----------------------------------
	char opcion[] = "0";						//Opcion del Menu principal
	char date[50];								//Buffer para la respuestas de sonda.


	char auxC[] = "cc.cc";						//Variable Campo Compuesto
	uint8_t longitudC = sizeof(auxC);
	uint8_t cantmedicion = 0;					//contador de mediciones correctar recibidas.


	 // ----- Setup -----------------------------------
	boardInit();


	//-- configuracion UART
	uartConfig( UART_USB, 115200 ); // Inicializar UART_USB a 115200 baudios
	uartConfig( UART_232, 115200 ); // Inicializar UART_232 a 115200 baudios



	// ----- Repeat for ever -------------------------
	while( true ) {

		switch  (state) {

		case Base:					// Base inicial

			printf("MENU PRINCIPAL\r\n");
			printf("(1) Medir Campo\r\n");
			printf("(2) ---\r\n");
			printf("(3) --- \r\n");
			scanf("%s",opcion);
			printf("El valor ingresado: %s\r\n", opcion);
			state = Opciones;
			break;

		case Opciones:				// Opciones de trabajo

			if (opcion[0] == '1'){
				opcion[0]= '0'; 		//clear opcion
				state = Conectar;
				break;
				}

			if (opcion[0] == '2'){
				opcion[0]= '0'; 		//clear opcion
				// Agregar opcion 2
				state = Base;
				break;
			}

			if (opcion[0] == '3'){
				opcion[0]= '0'; 		//clear opcion
				// Agregar opcion 3
				state = Base;
				break;
			}

			state = Base;
			opcion[0]= '0'; 			//clear opcion
			break;

		case Conectar:								// Conectar SONDA

			printf("ENCENDER SONDA... \r\n");

			delay(100);

			uartTxWrite(UART_232, 'r');				// Comando encender sonda "r" -SONDA ON-
			uartTxWrite(UART_232, '\r');
			uartTxWrite(UART_232, '\n');

			date [0] = 0;							//clear respuesta de sonda


			while (uartRxReady(UART_232)) { 		//Hay datos no leidos?
				for (uint8_t i = 0; i < 50; i++){
					date[i]= uartRxRead(UART_232);  //leo la respuesta de sonda
					}
				}

			if ((date[0] == ':') && (date[1] == 'r')){  // Se verifica la respuesta de la sonda ":r" -SONDA OK-
				printf("ESTADO DE SONDA: ON\r\n");
				for (uint8_t i = 0; date[i] != '\0'; i++) {
					uartTxWrite(UART_USB, date[i]);
					}
				delay(1000); // Debo esperar al menos unos segundos hasta que se inicie la sonda
				state = Medir;
				}
			break;

		case Medir:							// Medicion SONDA

			printf("MEDIR CAMPO... \r\n");

			char patron[] = "N";						//Patron fin del string.
			uint16_t patronSize = sizeof(patron);

			char buffer[] = ":Dxx.xxyy.yyzz.zzcc.ccN";  //Buffer recepcion de caracteres. 23caracteres + '/0' = longitud 24
			uint32_t longitud = sizeof(buffer);

			char* receiveBuffer = buffer;				// es lo mismo que: *receiveBuffer = &buffer[0]; asigno al puntero (* receiveBuffer) la direccion del primer valor del buffer.
			uint32_t* receiveBufferSize = &longitud;	//  asigno al puntero (* receiveBufferSize) la direccion del valor longitud.

			tick_t timeout = 5000;  					//tiempo en miliseg de espera. 5Seg.

			uartTxWrite(UART_232, 'D');         //Comando D5 -> Medir Campo
			uartTxWrite(UART_232, '5');
			uartTxWrite(UART_232, '\r');
			uartTxWrite(UART_232, '\n');



			//Recibo y guardo caracteres hasta que llegue el caracter patron o finaliza por tiempoout.
			if (receiveBytesUntilReceiveStringOrTimeoutBlocking(UART_232,patron,patronSize,receiveBuffer,receiveBufferSize,timeout)){
				gpioToggle(LED1);
				state = Procesar;
				break;
				}
				else{												//Error por timeout o sin patron final
					gpioToggle(LED2);
					state = Error;
					break;
					}

			state = Medir;
			break;




		case Procesar:

			if(buffer[0]== ':' && buffer[1]== 'D'){				//Verificacion de recepcion ":D"
				gpioToggle(LED3);
				//cantmedicion++;
				for(uint32_t i=17 ; i<(longitud-1) ; i++ ){
					auxC[i-17]=buffer[i];
					uartTxWrite(UART_USB,auxC[i-17]);
					uartTxWrite(UART_USB,'\r');
					uartTxWrite(UART_USB,'\n');
					}
				state = Medir;
				}
				else{											//Error en verificacion de recepcion ":D"
					state = Error;
					break;
					}
			//state = Base;
			break;

		case Error:
			gpioToggle(LEDB);
			delay(100);
			uartWriteString( UART_USB, buffer );
			uartTxWrite(UART_USB,'\r');
			uartTxWrite(UART_USB,'\n');
			// tratar que paso cuando no llega el caracter patron N o se paso te tiempo. Ver los errores E1 E2..
			// en buffer siempre queda los caracteres recibidos se pueden analizar o descartar.
			state = Medir;
			break;

		default:
			state = Base;
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

// ----------------- Funciones auxiliares -------------------

void clearbuffer(char* buffer, uint32_t longitud){

	char standar[] = "00000000000000000000000"; // ":Dxx.xxyy.yyzz.zzcc.ccN";

	for(uint32_t i = 0;i<longitud;i++){
		buffer[i]= standar[i];
		}

}

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

