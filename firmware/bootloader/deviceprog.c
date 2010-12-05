/* deviceprog.c
 *
 * Routines for dealing with the onboard flash and EEPROM memory.
 */

#include "deviceprog.h"
#include "misc.h"
#include <avr/boot.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>


/* Program the given page from the data in 'buf'.  The buffer must have at least one full page of
 * data in it.
 */
void Flash_ProgramPage(uint16_t page, uint16_t *buf)
{
	uint16_t page_addr = page * SPM_PAGESIZE;
	uint16_t offset;

	cli();

	// write latches one by one
	for(offset = 0; offset < SPM_PAGESIZE; offset += 2)
		boot_page_fill(page_addr + offset, buf[offset >> 1]);

	boot_page_write(page_addr);
	boot_spm_busy_wait();
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

	for(offset = 0; offset < SPM_PAGESIZE; offset += 2)
	{
		if(pgm_read_word(page_addr + offset) != buf[offset >> 1])
			return false;
	}
	return true;
}

bool Flash_WriteAppInfo(appinfo_t *appinfo)
{
	uint8_t buf[SPM_PAGESIZE];
	uint16_t count = 0;
	uint16_t n;
	bool writeOK;

	// Covers the case of appinfo crossing page boundaries
	do
	{
		uint16_t pageaddr = ((APPINFO_ADDR + count) / SPM_PAGESIZE) * SPM_PAGESIZE;
		uint16_t pageoffset = (APPINFO_ADDR + count) % SPM_PAGESIZE;

		n = MIN(sizeof(appinfo_t) - count, SPM_PAGESIZE - pageoffset);

		memcpy_P(buf, (PGM_VOID_P)pageaddr, SPM_PAGESIZE);            // read page into buf
		memcpy(&buf[pageoffset], (PGM_VOID_P)appinfo + count, n);     // copy appinfo to buf

		count += n;

		Flash_ProgramPage(pageaddr / SPM_PAGESIZE, (uint16_t *)buf);
		writeOK = Flash_VerifyPage(pageaddr / SPM_PAGESIZE, (uint16_t *)buf);
	}
	while(count < sizeof(appinfo_t)  &&  writeOK);

	return writeOK;
}

/* Erase the application.
 */
void Flash_EraseApp()
{
	appinfo_t appinfo;
	uint16_t addr;
	uint16_t end_addr = APP_SPACE_END;
	
	GetAppInfo(&appinfo);

	if(APPINFO_VALID == appinfo.valid)
		end_addr = MIN(appinfo.num_pages * SPM_PAGESIZE, APP_SPACE_END);

	for(addr = APP_SPACE_START; addr < end_addr; addr += SPM_PAGESIZE)
		boot_page_erase(addr);
}

/* Erase everything in EEPROM space.  Use this only if some setting in EEPROM is causing your
 * program to fail.
 */
void EEPROM_EraseData()
{
	uint16_t i;

	for(i = 0; i < E2END; ++i)     // E2END from avr/io.h
		eeprom_write_byte((uint8_t *)i, 0xFF);
}
