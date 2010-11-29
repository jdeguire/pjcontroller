/* deviceprog.c
 *
 * Routines for dealing with the onboard flash and EEPROM memory.
 */

#include "deviceprog.h"
#include <stdint.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/crc16.h>


/* Program the given page from the data in 'buf'.  The buffer must have at least one full page of
 * data in it.  For the ATmega328P, one page contains 64 words (128 bytes).
 */
void Flash_ProgramPage(uint16_t page, uint16_t *buf)
{
	uint16_t page_addr = page * SPM_PAGESIZE;
	uint16_t offset;

	// write latches one by one
	for(offset = 0; offset < SPM_PAGESIZE; offset += 2)
	{
		cli();
		boot_page_fill(page_addr + offset, buf[offset >> 1]);
		sei();
	}

	cli();
	boot_page_write(page_addr);
	sei();

	boot_spm_busy_wait();

	cli();
	boot_rww_enable();        // so we can read back the flash to verify it
	sei();
}

/* Return True if the contents of the page match the contents of the buffer.  Use this to confirm
 * that the page was written correctly.
 */
bool Flash_VerifyPage(uint16_t page, uint16_t *buf)
{
	uint16_t page_addr = page * SPM_PAGESIZE;
	uint16_t offset;

	for(offset = 0; offset < SPM_PAGESIZE; ++offset)
	{
		if(pgm_read_byte(page_addr + offset) != buf[offset])
			return false;
	}
	return true;
}

/* Calculate 16-bit CRC for entire application space.
 */
uint16_t Flash_CalculateAppCRC()
{
	uint16_t crc = 0xFFFF;
	uint16_t addr;
	
	for(addr = APP_SPACE_START; addr <= APP_SPACE_END; ++addr)
		crc = _crc_ccitt_update(crc, pgm_read_byte(addr));

	return crc;
}

/* Erase the entire application space.
 */
void Flash_EraseApp()
{
	uint16_t addr = APP_ID_START;
	
	// erase checksum row first so that partial programming is seen as no app on board
	boot_page_erase(addr);

	for(addr = APP_SPACE_START; addr < APP_SPACE_END; addr += SPM_PAGESIZE)
		boot_page_erase(addr);
}

/* Erase everything in EEPROM space.  Use this only if some setting in EEPROM is causing your
 * program to fail.
 */
void EEPROM_EraseData()
{
	uint16_t i;

	for(i = 0; i < E2END; ++i)
		eeprom_write_byte(i, 0xFF);
}
