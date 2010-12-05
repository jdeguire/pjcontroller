/* main.c
 *
 * Main code module for the bootloader.
 */

#include "regmap.h"
#include "uart.h"
#include "sharedmem.h"
#include "deviceprog.h"
#include "watchdog.h"
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>


int main(void)
{
	bool led_on = AppRestartRequested();
	ClearAppRestartRequest();

	// LED on PC5
	DDRC |= (1 << DD5);       // 1 sets pin to output

	MCUCR = (1 << IVCE);
	MCUCR = (1 << IVSEL);     // set IVT to boot space
	sei();                    // enable interrupts
	UART_Init();
	
	if(led_on)
	{
		PORTCbits.pc5 = 1;
		UART_TxString("Yup\r");
	}
	else
	{
		PORTCbits.pc5 = 0;
		UART_TxString("Nope\r");
	}

	while(1)
	{
		_delay_ms(2000);
		if(led_on)
			RestartBootloader();
		else
			RestartApp();
	}
}
