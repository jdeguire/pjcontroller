/* main.c
 *
 * Main code module for the bootloader.
 */

#include "regmap.h"
#include "uart.h"
#include "bootcmd.h"
#include "sharedmem.h"
#include "deviceprog.h"
#include "watchdog.h"
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>


int main(void)
{
	bool led_on;

	wdt_enable(WDTO_250MS);

	led_on = AppRestartRequested();
	ClearAppRestartRequest();

	UART_Init();
	Cmd_InitInterface();

	if(ResetByWatchdog())
		UART_TxString("Watchdog Reset\r");

	// LED on PC5
	DDRC |= (1 << DD5);       // 1 sets pin to output

	MCUCR = (1 << IVCE);
	MCUCR = (1 << IVSEL);     // set IVT to boot space
	sei();                    // enable interrupts
	
	wdt_reset();

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
		wdt_reset();
		Cmd_ProcessInterface();
//		_delay_ms(2000);
//		if(led_on)
//			RestartBootloader();
//		else
//			RestartApp();
	}
}
