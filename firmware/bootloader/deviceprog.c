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
 * File:   deviceprog.c
 * Author: Jesse DeGuire
 *
 * Routines for dealing with the onboard flash and EEPROM memory.
 */

#include "deviceprog.h"
#include "regmap.h"
#include "watchdog.h"
#include <avr/boot.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>


/* Write the given instruction word to the write latch given by 'offset'.  'offset' is a byte
 * address and so should be an even number.  A latch can only be written to once before being
 * cleared by a write operation or by writing to the RWWSRE bit in the SPMCSR register.
 */
void Flash_WriteLatch(uint16_t offset, uint16_t insword)
{
	cli();
	boot_page_fill(offset, insword);
	sei();
}

/* Program the page containing 'addr' from the data contained in the device's internal write
 * latches.  Use Flash_WriteLatch() to fill these latches before writing.
 */
void Flash_ProgramPage(uint16_t addr)
{
	cli();
	boot_page_write(addr);
	boot_spm_busy_wait();
	boot_rww_enable();        // so we can read back the flash to verify it
	sei();
}

/* Return True if the contents of the page starting with 'addr' match what is in the given buffer.
 * 'addr' must be the starting address of the page.
 */
bool Flash_VerifyPage(uint16_t addr, uint16_t *buf)
{
	uint16_t offset;

	for(offset = 0; offset < SPM_PAGESIZE; offset += 2)
	{
		if(pgm_read_word(addr + offset) != buf[offset >> 1])
			return false;
	}
	return true;
}

/* Erase the application.  The erase operation erases all pages in app space.
 */
void Flash_EraseApp()
{
	uint16_t addr = APP_SPACE_START;

	// erase the app checksum first so that the bootloader won't start a partially-erased app
	boot_page_erase(FLASHEND - SPM_PAGESIZE + 1);
	
	for( ; addr < APP_SPACE_END; addr += SPM_PAGESIZE)
	{
		wdt_reset();
		boot_page_erase(addr);
	}
}

/* Erase everything in EEPROM space.  Use this only if some setting in EEPROM is causing your
 * program to fail.
 */
void EEPROM_EraseData()
{
	uint16_t i;
	uint8_t eecr_shadow = EECR;
	EECRbits.eepm = 1;             // erase-only mode

	for(i = 0; i <= E2END; ++i)
	{
		EEAR = i;
		EECRbits.eere = 1;

		if(0xFF != EEDR)
		{
			cli();
			EECRbits.eempe = 1;
			EECRbits.eepe = 1;
			sei();

			while(EECRbits.eepe)
			{}
		}

		wdt_reset();
	}

	EECR = eecr_shadow;
}
