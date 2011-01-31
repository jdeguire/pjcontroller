/* localcmd.c
 *
 * Serial interface commmands specific to the bootloader.  See common/cmd.c for the implementation
 * of the interface itself.
 */

#include "localcmd.h"
#include "cmd.h"
#include "deviceprog.h"
#include "sharedmem.h"
#include "uart.h"
#include "watchdog.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/io.h>


static void JumpToApp_CMD(const char *cmdbuf, uint8_t len) __attribute__((noreturn));
static void RestartBootloader_CMD(const char *cmdbuf, uint8_t len) __attribute__((noreturn));

#if 0
static const prog_char m_jumpapphelp[]   = "Jump to the application";
static const prog_char m_restartblhelp[] = "Restart bootloader";
static const prog_char m_eraseapphelp[]  = "Erase application";
static const prog_char m_eraseeephelp[]  = "Erase EEPROM data";
static const prog_char m_progpagehelp[]  = "Program an app page";
static const prog_char m_writecrchelp[]  = "Write app CRC to flash";
static const prog_char m_pagesizehelp[]  = "Get size of one page";
static const prog_char m_numpageshelp[]  = "Get number of app pages";
#endif

static void JumpToApp_CMD(const char *cmdbuf, uint8_t len)
{
	// Jump straight to app without resetting
	asm volatile("jmp 0x0");
	for(;;) ;
}

static void RestartBootloader_CMD(const char *cmdbuf, uint8_t len)
{
	RestartBootloader();
}

#warning Add confirmation arg for erase commands

static void EraseApp_CMD(const char* cmdbuf, uint8_t len)
{
	uint16_t buf[SPM_PAGESIZE / 2];
	uint16_t page_addr = (FLASHEND - SPM_PAGESIZE + 1);

	// invalidate app checksum by zeroing out location in which it is stored
	memset(buf, 0xFF, sizeof(buf) - 4);
	buf[SPM_PAGESIZE / 2 - 2] = 0;
	buf[SPM_PAGESIZE / 2 - 1] = 0;
	Flash_ProgramPage(page_addr, (uint16_t *)buf);

	Flash_EraseApp();
}

static void EraseEEPROM_CMD(const char* cmdbuf, uint8_t len)
{
	EEPROM_EraseData();
}


static void ProgramPage_CMD(const char* cmdbuf, uint8_t len)
{
	uint16_t page_addr;
	uint16_t expectedCSum;
	const char *startptr = cmdbuf + 3;   // two command chars and a space
	char *endptr;
	bool argsfound = false;
	
	page_addr = (uint16_t)strtoul(startptr, &endptr, 10) * SPM_PAGESIZE;

	if(endptr != startptr)
	{
		startptr = endptr;

#warning Verify page address is within bounds

		expectedCSum = (uint16_t)strtoul(startptr, &endptr, 16);

		if(endptr != startptr)
		{
			uint8_t  buf[SPM_PAGESIZE];      // macro is bytes per page
			uint8_t  count;
			uint16_t calculatedCSum = 0;

			argsfound = true;
			UART_TxChar(':');

			// start collecting page data
			for(count = 0; count < SPM_PAGESIZE; ++count)
			{
				wdt_reset();                 // watchdog acts as our timeout

				while(UART_RxAvailable() == 0)
					;
				
				buf[count] = (uint8_t)UART_RxChar();
				calculatedCSum += buf[count];
			}

			wdt_reset();

#warning Add error codes for program and checksum command

			// program page if checksums match
			if(calculatedCSum == expectedCSum)              // needs error
			{
				Flash_ProgramPage(page_addr, (uint16_t *)buf);
				Flash_VerifyPage(page_addr, (uint16_t *)buf);    // needs error
			}
		}
	}

//	if(false == argsfound)                                  // needs error

}

static void WriteCRC_CMD(const char* cmdbuf, uint8_t len)
{
	uint16_t buf[SPM_PAGESIZE / 2];
	uint16_t page_addr = (FLASHEND - SPM_PAGESIZE + 1);

	// read
	memcpy_P((void *)buf, (PGM_VOID_P)page_addr, SPM_PAGESIZE);

	// modify
	buf[SPM_PAGESIZE / 2 - 2] = APP_CHECKSUM_VALID;
	buf[SPM_PAGESIZE / 2 - 1] = CalculateAppCRC();

	// write and verify
	Flash_ProgramPage(page_addr, (uint16_t *)buf);
	Flash_VerifyPage(page_addr, (uint16_t *)buf);  // needs error
}

static void PageSize_CMD(const char* cmdbuf, uint8_t len)
{
	char str[8];

	utoa(SPM_PAGESIZE, str, 10);
	UART_TxString(str);
	UART_TxChar('\r');
}

static void NumAppPages_CMD(const char* cmdbuf, uint8_t len)
{
	char str[8];

	utoa(NUM_APP_PAGES, str, 10);
	UART_TxString(str);
	UART_TxChar('\r');
}

void RegisterBootloaderCommands()
{
#if 0
	Cmd_RegisterCommand("j",  JumpToApp_CMD,         m_jumpapphelp);
	Cmd_RegisterCommand("r",  RestartBootloader_CMD, m_restartblhelp);
	Cmd_RegisterCommand("ea", EraseApp_CMD,          m_eraseapphelp);
	Cmd_RegisterCommand("ee", EraseEEPROM_CMD,       m_eraseeephelp);
	Cmd_RegisterCommand("pp", ProgramPage_CMD,       m_progpagehelp);
	Cmd_RegisterCommand("wc", WriteCRC_CMD,          m_writecrchelp);
	Cmd_RegisterCommand("ps", PageSize_CMD,          m_pagesizehelp);
	Cmd_RegisterCommand("pn", NumAppPages_CMD,       m_numpageshelp);
#else
	Cmd_RegisterCommand("j",  JumpToApp_CMD,         NULL);
	Cmd_RegisterCommand("r",  RestartBootloader_CMD, NULL);
	Cmd_RegisterCommand("ea", EraseApp_CMD,          NULL);
	Cmd_RegisterCommand("ee", EraseEEPROM_CMD,       NULL);
	Cmd_RegisterCommand("pp", ProgramPage_CMD,       NULL);
	Cmd_RegisterCommand("wc", WriteCRC_CMD,          NULL);
	Cmd_RegisterCommand("ps", PageSize_CMD,          NULL);
	Cmd_RegisterCommand("pn", NumAppPages_CMD,       NULL);
#endif
}
