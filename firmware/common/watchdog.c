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
 * File:   watchdog.c
 * Author: Jesse DeGuire
 *
 * Utilities for the AVR's watchdog timer.
 */

#include "watchdog.h"
#include <stdint.h>
#include <avr/io.h>

static void get_mcusr(void) __attribute__((used, naked, section(".init3")));

// in .noinit because this is set before the initialization stuff happens
static uint8_t m_mcusr_mirror __attribute__ ((section (".noinit")));


/* The watchdog is still enabled after a watchdog reset, so this will disable it for us early on in
 * the startup process so that the device is not stuck in a reset loop.  This function and the above
 * variable declaration were adapted from the avr-libc manual here:
 * http://www.nongnu.org/avr-libc/user-manual/group__avr__watchdog.html
 */
static void get_mcusr(void)
{
	m_mcusr_mirror = MCUSR;
	MCUSR = 0;
	wdt_disable();
}

/* Return True if the device was reset by the watchdog timer.  This means that only the watchdog
 * reset flag, and no others, is set in the MCUSR register.
 */
bool ResetByWatchdog()
{
	return (m_mcusr_mirror == (1 << WDRF));
}
