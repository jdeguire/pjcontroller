/* uart.h
 * 
 * Contains functions for setting up the UART module and transacting data.  There are separate send
 * and receive circular buffers to store data and interrupts for receiving data into and sending
 * data out of the buffers.
 */

#ifndef INCLUDE_UART_H_
#define INCLUDE_UART_H_

#include "regmap.h"
#include "system.h"
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define _UART_RXIE(n)  (UCSR0Bbits.rxcie0 = n)
#define _UART_TXIE(n)  (UCSR0Bbits.udrie0 = n)

#ifndef BUADRATE
#  define BAUDRATE 57600
#endif

#ifndef UART_TX_BUFSIZE
#  define UART_TX_BUFSIZE 64
#endif

#ifndef UART_RX_BUFSIZE
#  define UART_RX_BUFSIZE 64
#endif

#define UART_TxString(str)   UART_TxData(str, strlen(str))
#define UART_TxString_P(str) UART_TxData_P(str, strlen_P(str))

void UART_Init();
bool UART_TxChar(char c);
uint16_t UART_TxData(const char *data, uint16_t len);
uint16_t UART_TxData_P(const prog_char *data, uint16_t len);
char UART_RxChar();
uint16_t UART_RxData(char *data, uint16_t len);
uint16_t UART_TxAvailable();
uint16_t UART_RxAvailable();

#define UART_Printf(fmt, args...)										\
	do																	\
	{																	\
		char buf_123xyz_[UART_TX_BUFSIZE];								\
		snprintf(buf_123xyz_, UART_TX_BUFSIZE, fmt, ##args);			\
		UART_TxData(buf_123xyz_, strlen(buf_123xyz_));					\
	} while(0)

/* The format string 'fmt' should be in program space when using this. */
#define UART_Printf_P(fmt, args...)										\
	do																	\
	{																	\
		char buf_123xyz_[UART_TX_BUFSIZE];								\
		snprintf_P(buf_123xyz_, UART_TX_BUFSIZE, fmt, ##args);			\
		UART_TxData(buf_123xyz_, strlen(buf_123xyz_));					\
	} while(0)


#endif // INCLUDE_UART_H_
