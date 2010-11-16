/* main.c
 *
 * Main code module for the bootloader.
 */

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include "regmap.h"


int main(void)
{
	volatile uint8_t test = 0;

	// LED on PC5
	DDRC |= (1 << DD5);       // 1 sets pin to output
	PORTCbits.pc5 = 0;        // start with output low

	TCCR0Bbits.cs0 = 5;       // multi-bit field
	test = TCCR0Bbits.cs0;

	while(1)
	{
		PINCbits.pinc5 = 1;   // toggle state of output pin
		_delay_ms(1000);
	}
}
