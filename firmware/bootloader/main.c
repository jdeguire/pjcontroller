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
 * Main code module for the bootloader.
 */

#include "regmap.h"
#include "uart.h"
#include "cmd.h"
#include "sharedmem.h"
#include "deviceprog.h"
#include "localcmd.h"
#include "watchdog.h"
#include "led.h"
#include <stdint.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <util/delay.h>


typedef enum
{
	eBootOK      = 0,
	eBootPinSet,
	eBootRestart,
	eBootNoApp,
	eBootCRC
} bootstatus_t;

static bootstatus_t m_status;


/* Get the state of the Bootloader Pin.  If True, stay in the bootloader without attempting to start
 * the app.  Otherwise, try to start the application.
 */
static bool BootloaderPinSet()
{
	// we'll use an external pull-down for this
	DDRDbits.ddd4 = 0;        // 0 for input
	return PINDbits.pind4;
}

/* Check if there is any reason why the bootloader should not start the app and sets 'm_status' to
 * indicate the reason.
 */
static void GetAppStartupStatus()
{
	m_status = eBootOK;

	if(BootloaderPinSet())
		m_status = eBootPinSet;
	else if(!AppRestartRequested())
		m_status = eBootRestart;
	else
	{
		uint16_t appkey;
		uint16_t appcrc;

		appkey = pgm_read_word(FLASHEND - 3);
		appcrc = pgm_read_word(FLASHEND - 1);

		if(appkey != APP_CHECKSUM_VALID)
			m_status = eBootNoApp;
		else if(appcrc != CalculateAppCRC())
			m_status = eBootCRC;
	}

	ClearAppRestartRequest();
}

/* Turn on or off LED used as a visual notice that the board is in the bootloader.
 */
static void LightBootloaderLED(bool enable)
{
	if(enable)
	{
		YELLOW_LED_DDR = 1;    // output
		YELLOW_LED_PORT = 1;   // turn on
	}
	else
	{
		// return to defaults
		YELLOW_LED_DDR = 0;
		YELLOW_LED_PORT = 0;
	}
}

/* True enables the bootloader IVT and False returns to the app's IVT.  Set the False before jumping
 * to the application.
 */
static void SetBootIVT(bool bootIVT)
{
	if(bootIVT)
	{
		MCUCR = (1 << IVCE);
		MCUCR = (1 << IVSEL);
	}
	else
	{
		MCUCR = (1 << IVCE);
		MCUCR = 0;
	}
}

/* Print a message stating why we're in the bootloader along with the value of m_status.  This is
 * done so that software can grab the number and print its own message.
 */
void PrintBootStatus()
{
	UART_TxChar('(');
	UART_TxHexByte((uint8_t)m_status);
	UART_TxChar(')');

	switch(m_status)
	{
		case eBootPinSet:
			UART_TxData_P(PSTR(" Bootloader pin set\r"), 20);
			break;
		case eBootRestart:
			UART_TxData_P(PSTR(" WDT reset\r"), 11);
			break;
		case eBootNoApp:
			UART_TxData_P(PSTR(" No app loaded\r"), 15);
			break;
		case eBootCRC:
			UART_TxData_P(PSTR(" Invalid CRC\r"), 13);
			break;
		default:
			break;
	}
}

/* Undo the modifications to registers that the bootloader did when starting up.  Do this before
 * jumping to the app after the bootloader has started up.
 */
void RewindSettings()
{
	cli();                    // disable interrupts
	wdt_reset();
	SetBootIVT(false);
	LightBootloaderLED(false);

	// initial UART register values
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	UCSR0B = 0;
	UCSR0A = 0;
	UBRR0H = 0;
	UBRR0L = 0;

	wdt_disable();
}


int main(void)
{
	GetAppStartupStatus();

	if(m_status == eBootOK)
		asm volatile("jmp 0x0");
	else
	{
		wdt_enable(WDTO_250MS);

		UART_Init();
		Cmd_InitInterface();
		RegisterBootloaderCommands();

		SetBootIVT(true);
		sei();                    // enable interrupts

		LightBootloaderLED(true);
		PrintBootStatus();
		wdt_reset();

		while(true)
		{
			wdt_reset();
			Cmd_ProcessInterface();
		}
	}
}
