/* main.c
 *
 * Main code module for the bootloader.
 */

#include "regmap.h"
#include "uart.h"
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>


int main(void)
{
	uint8_t count = 0;

	// LED on PC5
	DDRC |= (1 << DD5);       // 1 sets pin to output
	PORTCbits.pc5 = 0;        // start with output low

	MCUCR = (1 << IVCE);
	MCUCR = (1 << IVSEL);     // set IVT to boot space
	sei();                    // enable interrupts
	UART_Init();

	while(1)
	{
		UART_Printf("%u\r", count);
		++count;
		_delay_ms(1000);

		if(UART_RxChar())
			PINCbits.pinc5 = 1;   // toggle state of output pin
	}
}

