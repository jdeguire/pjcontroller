/* deviceprog.c
 *
 * Routines for dealing with the onboard flash and EEPROM memory.
 */

#include "deviceprog.h"
#include "misc.h"
#include "watchdog.h"
#include <avr/boot.h>
#include <avr/io.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

/* NOTE:
 * Flash operations use the Z register (R30:R31 pair) for the address.  Since the AVR's memory is
 * paged, we can think of a 16-bit address as split into two parts like so:
 * [Page Number][Offset Within Page]
 * where the number of bits for each depends on the size of the flash and the size of each page.
 * Filling in the AVR write latches seems to only require the offset, whereas doing a page write or
 * erase cares about only the page number and the offset bits are ignored.
 * Reading flash does care about both since page structure does not come into play.
 */


/* Program the page containing 'addr' from the data in 'buf'.  The buffer must have at least one
 * full page of data in it.
 */
void Flash_ProgramPage(uint16_t addr, uint16_t *buf)
{
	uint16_t offset;

	cli();

	// write latches one by one
	for(offset = 0; offset < SPM_PAGESIZE; offset += 2)
		boot_page_fill(offset, buf[offset >> 1]);

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
		boot_page_erase(addr);
}

/* Erase everything in EEPROM space.  Use this only if some setting in EEPROM is causing your
 * program to fail.
 */
void EEPROM_EraseData()
{
	uint16_t i;

	for(i = 0; i < E2END; ++i)     // E2END from avr/io.h
	{
		eeprom_update_byte((uint8_t *)i, 0xFF);
		wdt_reset();
	}
}
