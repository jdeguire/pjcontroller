/* uart.c
 * 
 * Contains functions for setting up the UART module and transacting data.  There are separate send
 * and receive circular buffers to store data and interrupts for receiving data into and sending
 * data out of the buffers.
 */

#include "regmap.h"
#include <stdint.h>

#ifndef BUADRATE
#  define BAUDRATE 57600
#endif

// these assume normal speed asynchronous mode
#define BAUD2REG (((F_CPU / (8 * BAUDRATE)) - 1) / 2)       // rounds to nearest baud reg value
#define REG2BAUD (F_CPU / (16 * (BAUD2REG + 1)))            // baud reg value to actual rate

#define BAUDERROR ((1000L*REG2BAUD / BAUDRATE) - 1000L)

#if (BAUDERROR > 20  ||  BAUDERROR < -20)
#  error Baud rate error greater than 2%.  See Section 19 of the Reference Manual.
#endif
