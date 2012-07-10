/* main.c
 *
 * Main module for the PJC controller application.
 */

#include "regmap.h"
#include "uart.h"
#include "cmd.h"
#include "watchdog.h"
#include "localcmd.h"
#include <stdbool.h>
#include <avr/interrupt.h>

int main(void)
{
	DDRDbits.ddd7 = 1;   // output
	PORTDbits.pd7 = 1;   // turn on

	wdt_enable(WDTO_250MS);

	UART_Init();
	Cmd_InitInterface();
	RegisterAppCommands();
	sei();                    // enable interrupts

	while(true)
	{
		wdt_reset();
		Cmd_ProcessInterface();
	}
}
