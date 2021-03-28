/*=============================================================================

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

	// Recordar en config.mk poner USE_NANO=n (usar newlib completa
	// en lugar de newlib nano) para tener soporte de flotantes en
	// printf(), sin embargo, esto causa que el programa ocupe MUCHA
	// mas RAM y FLASH.

	 // ----- Variables -----------------------------------
	state_t state = Base;						//Estado inicial de la maquina.

	// ----- Variables -----------------------------------
	char opcion[] = "0";						//Opcion del Menu principal
	char date[50];								//Buffer para la respuestas de sonda.


	char auxC[] = "cc.cc";						//Variable Campo Compuesto
	char auxR[] = "cccc";						//Variable Campo Compuesto
	char error[] = "EE";
	uint8_t longitudC = sizeof(auxC);
	uint8_t longitudR = sizeof(auxR);
	uint8_t escala = 4;							// factor de division para cc.cc
	float listvalor[10];
	uint8_t contador = 0;					//contador de mediciones correctar recibidas.
	float factor;


	 // ----- Setup -----------------------------------
	boardInit();



	//-- configuracion UART
	uartConfig( UART_USB, 115200 ); // Inicializar UART_USB a 115200 baudios
	uartConfig( UART_232, 115200); // Inicializar UART_232 a 115200 baudios



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
				}
				else{												//Error por timeout o sin patron final
					gpioToggle(LED2);
					state = Error;
					}
			break;

		case Procesar:

			if(buffer[0]== ':' && buffer[1]== 'D'){				//Verificacion de recepcion ":D"
				gpioToggle(LED3);

				for(uint32_t i=17 ; i<(longitud-1) ; i++ ){		//Conservo solo los caracteres del tipo "cc.cc" o ".cccc"
					auxC[i-17]=buffer[i];
					}

				for (uint8_t i = 0;i<longitudC;i++){
					if(auxC[i] == '.'){							//  i=1 -> /1000 | i=2 -> /100 | i=3 -> /10 |i=4 -> /1
						escala = i;
						}
					}
				//-- configuracion de variables para convertir char a int. -------------
				char *ptr;
				long int ret;

				switch (escala){
							case 0:										// .cccc -> i=0 -> /10000
								for(uint8_t i = 1;i<longitudC;i++){
									auxR[i-1]=auxC[i];
									}
								ret = strtol(auxR, &ptr, 10);				// de char a int.
								listvalor[contador]= ((float)ret)/(float)10000; // int a float ajustado a su escala
								printf("El valor ingresado caso 0: %.4f\r\n", listvalor[contador]);
								contador++;
								break;

							case 1:										 // c.ccc -> i=1 -> /1000
								for(uint8_t i = 0;i<escala;i++){
									auxR[i]=auxC[i];
									}
								for(uint8_t i = 2;i<longitudC;i++){
									auxR[i-1]=auxC[i];
									}
								ret = strtol(auxR, &ptr, 10);  					// de char a int.
								listvalor[contador]= ((float)ret)/(float)1000;; // int a float ajustado a su escala
								printf("El valor ingresado caso 1: %.4f\r\n", listvalor[contador]);
								contador++;
								break;

							case 2:										 // cc.cc -> i=2 -> /100
								for(uint8_t i = 0;i<escala;i++){
									auxR[i]=auxC[i];
									}
								for(uint8_t i = 3;i<longitudC;i++){
									auxR[i-1]=auxC[i];
									}
								ret = strtol(auxR, &ptr, 10);
								listvalor[contador]= (float)ret/(float)100; //

								printf("El valor ingresado caso 2 float: %.4f\r\n",listvalor[contador]);
								contador++;
								break;

							case 3:										// ccc.c -> i=3 -> /10
								for(uint8_t i = 0;i<escala;i++){
									auxR[i]=auxC[i];
									}
								for(uint8_t i = 4;i<longitudC;i++){
									auxR[i-1]=auxC[i];
									}
								ret = strtol(auxR, &ptr, 10);
								listvalor[contador]= ((float)ret)/(float)10;
								printf("El valor ingresado caso 3: %.4f\r\n",listvalor[contador]);
								contador++;
								break;

							case 4:										// cccc. i=4 -> /1
								for(uint8_t i = 0;i<longitudC-1;i++){
									auxR[i]=auxC[i];
									}
								ret = strtol(auxR, &ptr, 10);
								listvalor[contador]= ((float)ret)/(float)1;
								printf("El valor ingresado caso 4: %.4f\r\n", listvalor[contador]);
								contador++;
								break;
							default:
								break;
							}

				state = Medir;

				if(contador == 10){
					contador =0;
					float suma = listvalor[0];
					for(uint8_t i=1;i<10;i++){

						suma = (suma + listvalor[i]);
						}
					printf("El lista de valores: %.4f\r\n", suma);
					printf("El lista de valores: %.4f\r\n", suma/(float)10);
					state = Base;
					}

			}
			else{											//Error en verificacion de recepcion ":D"
				state = Error;
				}

			break;

		case Error:
			gpioToggle(LEDB);
			if(buffer[0]== 'E'){
			switch(buffer[1]){
				case '1':
					printf("ERROR DE COMUNICACION E1\r\n");
					break;
				case '2':
					printf("ERROR DE BUFFER E2\r\n");
					break;
				case '3':
					printf("COMANDO RECIBIDO NO VALIDO E3\r\n");
					break;
				case '4':
					printf("PARAMETRO RECIBIDO NO VALIDO E4\r\n");
					break;
				case '5':
					printf("ERROR DE HARDWARE E5\r\n");
					break;
				case '6':
					printf("ERROR DE PARIDAD E6\r\n");
					break;
				case '7':
					printf("COMANDO RECIBIDO INCORRECTO E7\r\n");
					break;
				case '8':
					printf("COMANDO NO DISPONIBLE E8\r\n");
					break;
				case '9':
					printf("COMANDO RECIBIDO INCORRECTO E9\r\n");
					break;

				default:
					printf("TIEMPO OUT..\r\n");
					uartWriteString( UART_USB, buffer );
					uartTxWrite(UART_USB,'\r');
					uartTxWrite(UART_USB,'\n');
					break;
				}
			}
			else {
				printf("TIEMPO OUT..\r\n");
				uartWriteString( UART_USB, buffer );
				uartTxWrite(UART_USB,'\r');
				uartTxWrite(UART_USB,'\n');
				}
			state = Medir;
			break;

		default:
			state = Base;
			break;

		}
	}

   // YOU NEVER REACH HERE, because this program runs directly or on a
   // microcontroller and is not called by any Operating System, as in the 
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

