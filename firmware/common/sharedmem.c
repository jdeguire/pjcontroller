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
 * File:   sharedmem.c
 * Author: Jesse DeGuire
 *
 * Used for routines and memory shared between the PJC Bootloader and the application.
 */

#include "sharedmem.h"
#include "watchdog.h"
#include "misc.h"
#include <util/crc16.h>
#include <avr/interrupt.h>

// Not initialized by the startup module, so that memory contents are retained after a watchdog
// reset.  Note that the memory state is not trustworthy for other reset sources.
static volatile bool m_loadApplication __attribute__((section(".shareddata")));


/* Return True if the application reset the device (using the watchdog) and requested that the
 * bootloader restart the app.  If the device was _not_ reset by the watchdog, then restart the app
 * so that powering up the device runs the app as exptected.
 */
bool AppRestartRequested()
{
	return (!ResetByWatchdog()  ||  (ResetByWatchdog()  &&  m_loadApplication));
}

/* Clear the request to restart the app on reset.  This is done in the bootloader so that an
 * unintentional watchdog reset will land in the bootloader rather than restarting the (probably)
 * faulty application.
 */
void ClearAppRestartRequest()
{
	m_loadApplication = false;
}

/* Restart the application by requesting the bootloader to restart the app, enabling the shortest
 * watchdog time, and looping until the device restarts.  Note that other conditions (such as a bad
 * checksum) can still cause the bootloader to not start the app.
 */
void RestartApp()
{
	cli();
	m_loadApplication = true;
    wdt_enable(WDTO_15MS);
	for(;;) ;
}

/* Restart and stay in the bootloader by enabling the shortest watchdog timer and running an
 * infinite loop.
 */
void RestartBootloader()
{
	cli();
	m_loadApplication = false;
    wdt_enable(WDTO_15MS);
	for(;;)	;
}

/* Calculate 16-bit CRC for everything in app space.
 */
uint16_t CalculateAppCRC()
{
	uint16_t crc = 0xFFFF;
	uint16_t addr;

	wdt_reset();

	for(addr = APP_SPACE_START; addr <= APP_SPACE_END; ++addr)
		crc = _crc_ccitt_update(crc, pgm_read_byte(addr));

	wdt_reset();

	return crc;
}
