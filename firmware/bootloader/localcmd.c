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
#include <avr/boot.h>

// defined in main.c
void RewindSettings();
void PrintBootStatus();

static void JumpToApp_CMD(const char *cmdbuf, uint8_t len) __attribute__((noreturn));
static void RestartBootloader_CMD(const char *cmdbuf, uint8_t len) __attribute__((noreturn));

typedef enum
{
	eCmdOK = 0,          // no error; used to indicate success
	eCmdErrArgs,         // invalid or missing arguments
	eCmdErrPageOOB,      // page out of bounds
	eCmdErrChecksum,     // expected checksum does not match calculated
	eCmdErrVerify        // page verify failed
} cmdstatus_t;


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
	cmdstatus_t status = eCmdErrArgs;

	if(0 == strncmp(cmdbuf + 3, "yes", 3))
	{
		Flash_EraseApp();
		status = eCmdOK;
	}

	PrintCmdStatus(status);
}

static void EraseEEPROM_CMD(const char* cmdbuf, uint8_t len)
{
	cmdstatus_t status = eCmdErrArgs;

	if(0 == strncmp(cmdbuf + 3, "yes", 3))
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
	cmdstatus_t status = eCmdOK;

	memset(buf, 0xFF, sizeof(buf) - 4);

	// The CRC and validation key go into the last words of flash
	buf[SPM_PAGESIZE / 2 - 2] = APP_CHECKSUM_VALID;
	buf[SPM_PAGESIZE / 2 - 1] = CalculateAppCRC();

	Flash_ProgramPage(FLASHEND - SPM_PAGESIZE + 1, (uint16_t *)buf);

	if(!Flash_VerifyPage(FLASHEND - SPM_PAGESIZE + 1, (uint16_t *)buf))
		status = eCmdErrVerify;

	PrintCmdStatus(status);
}

static void PageSize_CMD(const char* cmdbuf, uint8_t len)
{
	UART_TxHexByte((SPM_PAGESIZE >> 8) & 0xFF);
	UART_TxHexByte(SPM_PAGESIZE & 0xFF);
	UART_TxChar('\r');
}

static void NumAppPages_CMD(const char* cmdbuf, uint8_t len)
{
	UART_TxHexByte((NUM_APP_PAGES >> 8) & 0xFF);
	UART_TxHexByte(NUM_APP_PAGES & 0xFF);
	UART_TxChar('\r');
}

static void PrintBootStatus_CMD(const char* cmdbuf, uint8_t len)
{
	PrintBootStatus();
}


void RegisterBootloaderCommands()
{
	Cmd_RegisterCommand("j",  JumpToApp_CMD,         PSTR("Jump to app"));
	Cmd_RegisterCommand("r",  RestartBootloader_CMD, PSTR("Restart bootloader"));
	Cmd_RegisterCommand("ea", EraseApp_CMD,          PSTR("Erase app"));
	Cmd_RegisterCommand("ee", EraseEEPROM_CMD,       PSTR("Erase EEPROM"));
	Cmd_RegisterCommand("pp", ProgramPage_CMD,       PSTR("Program page"));
	Cmd_RegisterCommand("wc", WriteCRC_CMD,          PSTR("Write app CRC"));
	Cmd_RegisterCommand("ps", PageSize_CMD,          PSTR("Get page size"));
	Cmd_RegisterCommand("pn", NumAppPages_CMD,       PSTR("Get number of app pages"));
	Cmd_RegisterCommand("s",  PrintBootStatus_CMD,   PSTR("Get bootup status"));
}
