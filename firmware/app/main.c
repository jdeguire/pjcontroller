/* Copyright © 2011-2013 Jesse DeGuire
 *
 * This file is part of Projector Controller.
 *
 * Projector Controller is free software: you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Projector Controller is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with Projector Controller.  If not, see <http://www.gnu.org/licenses/>.
 *
 * File:   main.c
 * Author: Jesse DeGuire
 *
 * Main module for the PJC controller application.
 */

#include "regmap.h"
#include "uart.h"
#include "cmd.h"
#include "led.h"
#include "watchdog.h"
#include "localcmd.h"
#include <stdbool.h>
#include <avr/interrupt.h>

int main(void)
{
	GREEN_LED_DDR = 1;   // output
	GREEN_LED_PORT = 1;  // turn on

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
