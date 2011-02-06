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
#include "misc.h"
#include "watchdog.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/io.h>

// defined in main.c
void RewindSettings();
void PrintBootStatus();

static void JumpToApp_CMD(const char *cmdbuf, uint8_t len) __attribute__((noreturn));
static void RestartBootloader_CMD(const char *cmdbuf, uint8_t len) __attribute__((noreturn));

typedef enum
{
	eCmdOK          = 0x80,    // no error; used to indicate success
	eCmdErrArgs,               // invalid or missing arguments
	eCmdErrPageOOB,            // page out of bounds
	eCmdErrChecksum,           // expected checksum does not match calculated
	eCmdErrVerify              // page verify failed
} cmdstatus_t;

static const prog_char m_jumpapphelp[]   = "Jump to app";
static const prog_char m_restartblhelp[] = "Restart bootloader";
static const prog_char m_eraseapphelp[]  = "Erase app";
static const prog_char m_eraseeephelp[]  = "Erase EEPROM";
static const prog_char m_progpagehelp[]  = "Program page";
static const prog_char m_writecrchelp[]  = "Write app CRC";
static const prog_char m_pagesizehelp[]  = "Get page size";
static const prog_char m_numpageshelp[]  = "Get number of app pages";
static const prog_char m_statushelp[]    = "Get bootup status";


static uint16_t htou(const char *str, const char **endptr)
{
	uint16_t val   = 0;
    uint16_t count = 0;
	bool     done  = false;

	if(endptr)
		*endptr = str;

	while(*str == ' ')
		++str;

	while(!done)
	{
		if((uint8_t)(*str - '0') < 10)
			val = (val << 4) | (uint8_t)(*str - '0');
		else if((uint8_t)(*str - 'a') < 6)
			val = (val << 4) | (uint8_t)(*str - 'a' + 10);
		else if((uint8_t)(*str - 'A') < 6)
			val = (val << 4) | (uint8_t)(*str - 'A' + 10);
		else
			done = true;

		++str;
		++count;
	}

	if(endptr  &&  count > 1)
		*endptr = str - 1;

	return val;
}

/* Print the result of an executed command.  Done so that all commands have a response and so that
 * they can indicate errors.
 */
static void PrintCmdStatus(cmdstatus_t status)
{
	UART_TxChar('!');
	UART_TxHexByte((uint8_t)status);
	UART_TxChar('\r');
}


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

static void EraseApp_CMD(const char* cmdbuf, uint8_t len)
{
	uint16_t buf[SPM_PAGESIZE / 2];
	cmdstatus_t status = eCmdErrArgs;

	if(0 == strcmp(cmdbuf + 3, "yes"))
	{
		// invalidate app checksum by zeroing out location in which it is stored
		memset(buf, 0xFF, sizeof(buf) - 4);
		buf[SPM_PAGESIZE / 2 - 2] = 0;
		buf[SPM_PAGESIZE / 2 - 1] = 0;
		Flash_ProgramPage((FLASHEND - SPM_PAGESIZE + 1), (uint16_t *)buf);

		Flash_EraseApp();
		
		status = eCmdOK;
	}

	PrintCmdStatus(status);
}

static void EraseEEPROM_CMD(const char* cmdbuf, uint8_t len)
{
	cmdstatus_t status = eCmdErrArgs;

	if(0 == strcmp(cmdbuf + 3, "yes"))
	{
		EEPROM_EraseData();
		status = eCmdOK;
	}

	PrintCmdStatus(status);
}

static void ProgramPage_CMD(const char* cmdbuf, uint8_t len)
{
 	uint8_t buf[SPM_PAGESIZE];      // bytes per page
	uint16_t csum;
	uint16_t page;
	uint16_t count;
	const char *endptr;
	cmdstatus_t status = eCmdOK;

	cmdbuf += 3;    // two command chars and a space

	page = htou(cmdbuf, &endptr);
	cmdbuf = endptr;

	csum = htou(cmdbuf, &endptr);

	// one of the args is missing if these are equal
	if(endptr != cmdbuf)
	{
		if(page < NUM_APP_PAGES)
		{
			// signal PC to start sending data and start collecting
			UART_TxChar(':');

			for(count = 0; count < SPM_PAGESIZE; ++count)
			{
				wdt_reset();                 // watchdog acts as our timeout

				while(UART_RxAvailable() == 0)
					;

				buf[count] = (uint8_t)UART_RxChar();
				csum -= buf[count];
			}

			wdt_reset();

			if(0 == csum)
			{
				Flash_ProgramPage(page * SPM_PAGESIZE, (uint16_t *)buf);

				if(!Flash_VerifyPage(page * SPM_PAGESIZE, (uint16_t *)buf))
					status = eCmdErrVerify;
			}
			else
				status = eCmdErrChecksum;
		}
		else
			status = eCmdErrPageOOB;
	}
	else
		status = eCmdErrArgs;

	PrintCmdStatus(status);
}

static void WriteCRC_CMD(const char* cmdbuf, uint8_t len)
{
	uint16_t buf[SPM_PAGESIZE / 2];
	uint16_t page_addr = (FLASHEND - SPM_PAGESIZE + 1);
	cmdstatus_t status = eCmdOK;

	// read
	memcpy_P((void *)buf, (PGM_VOID_P)page_addr, SPM_PAGESIZE);

	// modify
	buf[SPM_PAGESIZE / 2 - 2] = APP_CHECKSUM_VALID;
	buf[SPM_PAGESIZE / 2 - 1] = CalculateAppCRC();

	// write and verify
	Flash_ProgramPage(page_addr, (uint16_t *)buf);

	if(!Flash_VerifyPage(page_addr, (uint16_t *)buf))
		status = eCmdErrVerify;

	PrintCmdStatus(status);
}

static void PageSize_CMD(const char* cmdbuf, uint8_t len)
{
	union
	{
		uint16_t val;
		uint8_t  by[2];
	} pgsize;

	pgsize.val = SPM_PAGESIZE;

	UART_TxHexByte(pgsize.by[1]);
	UART_TxHexByte(pgsize.by[0]);
	UART_TxChar('\r');
}

static void NumAppPages_CMD(const char* cmdbuf, uint8_t len)
{
	union
	{
		uint16_t val;
		uint8_t  by[2];
	} numpages;

	numpages.val = NUM_APP_PAGES;

	UART_TxHexByte(numpages.by[1]);
	UART_TxHexByte(numpages.by[0]);
	UART_TxChar('\r');
}

static void PrintBootStatus_CMD(const char* cmdbuf, uint8_t len)
{
	PrintBootStatus();
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
	Cmd_RegisterCommand("s",  PrintBootStatus_CMD,   m_statushelp);
}
