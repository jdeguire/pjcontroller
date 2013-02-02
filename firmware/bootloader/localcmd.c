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
 * File:   localcmd.c
 * Author: Jesse DeGuire
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
	eCmdErrRange,        // argument out of range (ex: page outside address range)
	eCmdErrMismatch,     // value (ex: checksum or page data) does not match expected
} cmdstatus_t;

uint8_t m_pagebuf[SPM_PAGESIZE];


/* Convert a string of hex characters pointed to by 'str' into an unsigned integer.  If 'endptr' is
 * non-NULL, it will point to the character after the last-converted digit in 'str' if any hex
 * characters were converted; otherwise, it will be set equal to 'str'.  Use 'maxchars' to limit the
 * number of hex characters this function will process.  This function will move past any initial
 * spaces in 'str'.
 */
static uint16_t htou(const char *str, const char **endptr, uint8_t maxchars)
{
	uint16_t val   = 0;
	bool     done  = false;

	if(endptr)
		*endptr = str;

	while(*str == ' ')
		++str;

	while(!done  &&  maxchars)
	{
		if((uint8_t)(*str - '0') < 10)
		{
			val = (val << 4) | (uint8_t)(*str - '0');
			++str;
		}
		else if((uint8_t)(*str - 'a') < 6)
		{
			val = (val << 4) | (uint8_t)(*str - 'a' + 10);
			++str;
		}
		else if((uint8_t)(*str - 'A') < 6)
		{
			val = (val << 4) | (uint8_t)(*str - 'A' + 10);
			++str;
		}
		else
			done = true;

		--maxchars;
	}

	if(endptr  &&  *endptr != str)
		*endptr = str;

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


/* Jump to the application, which is at the start of flash.  This does not make sure a valid app is
 * there.
 * Syntax: j
 * Response: none (will see startup message after jump)
 */
static void JumpToApp_CMD(const char *cmdbuf, uint8_t len)
{
	RewindSettings();
	asm volatile("jmp 0x0");
	for(;;) ;
}

/* Reset the device and stay in the bootloader when it restarts.
 * Syntax: r
 * Response: none (will see startup message after reset)
 */
static void RestartBootloader_CMD(const char *cmdbuf, uint8_t len)
{
	RestartBootloader();
}

/* Erase the application on board.
 * Syntax: ea yes (the "yes" is required as a confirmation)
 * Response: status code
 */
static void EraseApp_CMD(const char* cmdbuf, uint8_t len)
{
	cmdstatus_t status = eCmdErrArgs;

	// "ea yes" is 6 chars
	if(len >= 6  &&  0 == strncmp(cmdbuf + 3, "yes", 3))
	{
		Flash_EraseApp();
		status = eCmdOK;
	}

	PrintCmdStatus(status);
}

/* Erase onboard EEPROM memory.
 * Syntax: ee yes (the "yes" is required as a confirmation)
 * Response: status code
 */
static void EraseEEPROM_CMD(const char* cmdbuf, uint8_t len)
{
	cmdstatus_t status = eCmdErrArgs;

	// "ee yes" is 6 chars
	if(len >= 6  &&  0 == strncmp(cmdbuf + 3, "yes", 3))
	{
		EEPROM_EraseData();
		status = eCmdOK;
	}

	PrintCmdStatus(status);
}

/* Write data to the page buffer, which will be programmed to flash when the Program Page command is
 * run.
 * Syntax: pd [offset] [checksum] [data bytes]
 * Response: status code
 */
static void LoadPageData_CMD(const char* cmdbuf, uint8_t len)
{
	uint16_t offset;
	uint16_t csum;
	const char *endptr;
	cmdstatus_t status = eCmdOK;

	cmdbuf += 3;    // two command chars and a space

	offset = htou(cmdbuf, &endptr, 255);
	cmdbuf = endptr;

	csum = htou(cmdbuf, &endptr, 255);

	// one of the args is missing if these are equal
	if(endptr != cmdbuf)
	{
		if(offset < SPM_PAGESIZE)
		{
			while(endptr != cmdbuf  &&  offset < SPM_PAGESIZE)
			{
				cmdbuf = endptr;
				m_pagebuf[offset] = (uint8_t)htou(cmdbuf, &endptr, 2);
				csum -= m_pagebuf[offset];
				++offset;
			}

			if(0 != csum)
				status = eCmdErrMismatch;
		}
		else
			status = eCmdErrRange;
	}
	else
		status = eCmdErrArgs;

	PrintCmdStatus(status);
}

/* Write the data loaded using the Load Page Data command to flash.
 * Syntax: pp [pagenum]
 * Response: status code
 */
static void ProgramPage_CMD(const char* cmdbuf, uint8_t len)
{
	uint16_t page;
	const char *endptr;
	cmdstatus_t status = eCmdOK;

	cmdbuf += 3;                        // two command chars and a space
	page = htou(cmdbuf, &endptr, 255);

	// the arg is missing if these are equal
	if(endptr != cmdbuf)
	{
		if(page < NUM_APP_PAGES)
		{
			uint16_t i = 0;

			for( ; i < SPM_PAGESIZE; i += 2)
				Flash_WriteLatch(i, *(uint16_t *)&m_pagebuf[i]);

			Flash_ProgramPage(page * SPM_PAGESIZE);

			if(!Flash_VerifyPage(page * SPM_PAGESIZE, (uint16_t *)m_pagebuf))
				status = eCmdErrMismatch;
		}
		else
			status = eCmdErrRange;
	}
	else
		status = eCmdErrArgs;

	memset(m_pagebuf, 0xFF, sizeof(m_pagebuf));

	PrintCmdStatus(status);
}

/* Calculate and write the application's CRC to flash memory.  This must be the last step in
 * programming a new application on the board.
 * Syntax: wc
 * Response: status code
 */
static void WriteCRC_CMD(const char* cmdbuf, uint8_t len)
{
	cmdstatus_t status = eCmdOK;
	uint16_t appcrc = CalculateAppCRC();

	// The CRC and validation key go into the last words of flash
	Flash_WriteLatch(SPM_PAGESIZE - 4, APP_CHECKSUM_VALID);
	Flash_WriteLatch(SPM_PAGESIZE - 2, appcrc);
	Flash_ProgramPage(FLASHEND);

	if(pgm_read_word(FLASHEND - 3) != APP_CHECKSUM_VALID  ||  
	   pgm_read_word(FLASHEND - 1) != appcrc)
	{
		status = eCmdErrMismatch;
	}

	PrintCmdStatus(status);
}

/* Output the number of bytes in a single page.
 * Syntax: ps
 * Response: uint16_t
 */
static void PageSize_CMD(const char* cmdbuf, uint8_t len)
{
	UART_TxHexByte((SPM_PAGESIZE >> 8) & 0xFF);
	UART_TxHexByte(SPM_PAGESIZE & 0xFF);
	UART_TxChar('\r');
}

/* Outputs the number of pages available for use by the application.
 * Syntax: pn
 * Response: uint16_t
 */
static void NumAppPages_CMD(const char* cmdbuf, uint8_t len)
{
	UART_TxHexByte((NUM_APP_PAGES >> 8) & 0xFF);
	UART_TxHexByte(NUM_APP_PAGES & 0xFF);
	UART_TxChar('\r');
}

/* Outputs a status message along with a numerical value stating why the device is running the
 * bootloader instead of the application.  The number allows an application to read that and output
 * its own message (useful for localization).
 * Syntax: s
 * Response: string
 */
static void PrintBootStatus_CMD(const char* cmdbuf, uint8_t len)
{
	PrintBootStatus();
}


/* Call this once at startup to register the above command handlers and associate that with commands
 * to be called from a serial console.
 */
void RegisterBootloaderCommands()
{
	memset(m_pagebuf, 0xFF, sizeof(m_pagebuf));

	Cmd_RegisterCommand("j",  JumpToApp_CMD,         PSTR("Jump to app"));
	Cmd_RegisterCommand("r",  RestartBootloader_CMD, PSTR("Restart bootloader"));
	Cmd_RegisterCommand("ea", EraseApp_CMD,          PSTR("Erase app"));
	Cmd_RegisterCommand("ee", EraseEEPROM_CMD,       PSTR("Erase EEPROM"));
	Cmd_RegisterCommand("pd", LoadPageData_CMD,      PSTR("Load page data"));
	Cmd_RegisterCommand("pp", ProgramPage_CMD,       PSTR("Program page"));
	Cmd_RegisterCommand("wc", WriteCRC_CMD,          PSTR("Write app CRC"));
	Cmd_RegisterCommand("ps", PageSize_CMD,          PSTR("Get page size"));
	Cmd_RegisterCommand("pn", NumAppPages_CMD,       PSTR("Get number of app pages"));
	Cmd_RegisterCommand("s",  PrintBootStatus_CMD,   PSTR("Get bootup status"));
}
