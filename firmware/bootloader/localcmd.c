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

// defined in main.c
void RewindSettings();

static void JumpToApp_CMD(const char *cmdbuf, uint8_t len) __attribute__((noreturn));
static void RestartBootloader_CMD(const char *cmdbuf, uint8_t len) __attribute__((noreturn));

static const prog_char m_jumpapphelp[]   = "Jump to app";
static const prog_char m_restartblhelp[] = "Restart bootloader";
static const prog_char m_eraseapphelp[]  = "Erase app";
static const prog_char m_eraseeephelp[]  = "Erase EEPROM";
static const prog_char m_progpagehelp[]  = "Program page";
static const prog_char m_writecrchelp[]  = "Write app CRC";
static const prog_char m_pagesizehelp[]  = "Get page size";
static const prog_char m_numpageshelp[]  = "Get number of app pages";


static void JumpToApp_CMD(const char *cmdbuf, uint8_t len)
{
	RewindSettings();
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
	uint8_t  buf[SPM_PAGESIZE];      // macro is bytes per page
	uint16_t calculatedCSum, expectedCSum;
	uint16_t page;
	char *csumptr;
	uint8_t  count;

	cmdbuf += 3;    // two command chars and a space
	page = (uint16_t)atol(cmdbuf);

	csumptr = strchr(cmdbuf, ' ');      // find next space before csum arg
	
	if(csumptr != NULL  &&  csumptr != cmdbuf)
	{
#warning Verify page address is within bounds

		expectedCSum = (uint16_t)atol(csumptr);
		calculatedCSum = 0;

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
			Flash_ProgramPage(page * SPM_PAGESIZE, (uint16_t *)buf);
			Flash_VerifyPage(page * SPM_PAGESIZE, (uint16_t *)buf);    // needs error
		}
	}
//  else                                     // missing args, needs error

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
	Cmd_RegisterCommand("j",  JumpToApp_CMD,         m_jumpapphelp);
	Cmd_RegisterCommand("r",  RestartBootloader_CMD, m_restartblhelp);
	Cmd_RegisterCommand("ea", EraseApp_CMD,          m_eraseapphelp);
	Cmd_RegisterCommand("ee", EraseEEPROM_CMD,       m_eraseeephelp);
	Cmd_RegisterCommand("pp", ProgramPage_CMD,       m_progpagehelp);
	Cmd_RegisterCommand("wc", WriteCRC_CMD,          m_writecrchelp);
	Cmd_RegisterCommand("ps", PageSize_CMD,          m_pagesizehelp);
	Cmd_RegisterCommand("pn", NumAppPages_CMD,       m_numpageshelp);
}
