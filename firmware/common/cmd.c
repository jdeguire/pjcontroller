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
 * File:   cmd.c
 * Author: Jesse DeGuire
 *
 * Command interface for talking over the serial port to the application or bootloader.  Common
 * commands are defined in here along with a function allowing you to add your own.
 */

#include "uart.h"
#include "sharedmem.h"
#include "cmd.h"
#include "watchdog.h"
#include <string.h>
#include <stdlib.h>

static void CalcAppCRC_CMD(const char *cmdbuf, uint8_t len);
static void PrintHelp_CMD(const char *cmdbuf, uint8_t len);
static void ShowVersion_CMD(const char *cmdbuf, uint8_t len)  __attribute__((noinline));

static const __flash char m_versionstr[] = VERSION_STRING "\r";

static char m_cmdbuf[CMD_BUFSIZE+1];    // one extra for null terminator
static uint8_t m_cmdlen;

static enum cmdstate_t
{
	eCmd_Prompt = 0,
	eCmd_Receive,
	eCmd_Run,
	eCmd_Help
} m_cmdstate;

static uint8_t m_helpindex;             // which command we're printing help for

static cmdinfo_t m_cmds[CMD_MAXCMDS];
static uint8_t m_numcmds = 0;


/* Initialize the interface by printing out version info, setting up the interface state machine,
 * and registering common commands.
 */
void Cmd_InitInterface()
{
	m_cmdstate = eCmd_Prompt;
	m_helpindex = 0;
	UART_TxString_P(PSTR("\r\r---Startup---\r"));
	UART_TxString_P(m_versionstr);

	Cmd_RegisterCommand("crc", CalcAppCRC_CMD,  PSTR("Calculate app CRC"));
	Cmd_RegisterCommand("h",   PrintHelp_CMD,   PSTR("Print help"));
	Cmd_RegisterCommand("v",   ShowVersion_CMD, PSTR("Show version"));
}

/* Add a new interface command that can be called via the connected serial console.  Parameters are
 * the serial command, the function that the command will call, and a short help string for the
 * command.  The maximum number of commands is deteremined by the CMD_MAXCMDS macro.  This does not
 * check for duplicate command names.
 *
 * Return True if a new command was added and False if it couldn't be added.
 */
bool Cmd_RegisterCommand(const char *cmdname, cmdhandler_t cmdfunc, const __flash char *help)
{
	bool result = false;

	if(m_numcmds < CMD_MAXCMDS)
	{
		m_cmds[m_numcmds].name = cmdname;
		m_cmds[m_numcmds].cmdfunc = cmdfunc;
		m_cmds[m_numcmds].help = help;
		++m_numcmds;
		result = true;
	}
	return result;
}

/* Run one iteration of the interface state machine.  This is responsible for displaying the command
 * prompt, receiving characters, and running the resulting command by searching for and calling the
 * proper command function.  Call this in your main loop.
 */
void Cmd_ProcessInterface()
{
	switch(m_cmdstate)
	{
		case eCmd_Prompt:
			if(UART_TxAvailable() >= 4)
			{
				UART_TxData("\r#> ", 4);
				m_cmdlen = 0;
				++m_cmdstate;
			}
			break;
		case eCmd_Receive:			
		{
			uint16_t avail = UART_RxAvailable();

			while(avail--)
			{
				char c = UART_RxChar();
				wdt_reset();

				if(0x08 == c)           // backspace -- remove last character
				{
					if(m_cmdlen > 0)
						--m_cmdlen;
				}
				else if(0x1B == c)      // escape -- clear prompt
				{
					m_cmdstate = eCmd_Prompt;
					break;
				}
				else                    // add the character to the buffer
				{
					m_cmdbuf[m_cmdlen] = c;
					++m_cmdlen;

					// either got a command or buffer is full
					if(c == '\r'  ||  m_cmdlen >= CMD_BUFSIZE)
					{
						m_cmdbuf[m_cmdlen] = 0;   // terminate command
						++m_cmdstate;
						break;
					}
				}
			}

			wdt_reset();
		}
		break;
		case eCmd_Run:
			m_cmdstate = eCmd_Prompt;

			if(m_numcmds > 0)
			{
				uint8_t i;

				// look for the right command and call its function
				for(i = 0; i < m_numcmds; ++i)
				{
					size_t len = strlen(m_cmds[i].name);

					if(0 == strncmp(m_cmds[i].name, m_cmdbuf, len)  &&
					   (m_cmdbuf[len] == ' '  ||  m_cmdbuf[len] == '\r'))
					{
						m_cmds[i].cmdfunc(m_cmdbuf, m_cmdlen);
						break;
					}
				}

				// didn't find the command
				if(i == m_numcmds)
					UART_TxData_P(PSTR("Unknown command\r"), 16);
			}
			break;
		case eCmd_Help:
			if(m_helpindex < m_numcmds)
			{
				size_t txlen = (strlen(m_cmds[m_helpindex].name) + 
								strlen_P(m_cmds[m_helpindex].help) + 5);

				if(txlen > UART_TX_BUFSIZE)
					txlen = UART_TX_BUFSIZE;

				if(UART_TxAvailable() >= txlen)
				{
					UART_TxString(m_cmds[m_helpindex].name);
					UART_TxData("\t - ", 4);
					UART_TxString_P(m_cmds[m_helpindex].help);
					UART_TxChar('\r');

					++m_helpindex;
				}
			}
			else
			{
				m_cmdstate = eCmd_Prompt;
				m_helpindex = 0;
			}
			break;
		default:
			m_cmdstate = eCmd_Prompt;
			break;
	}
}


/* The following functions are command functions.  Each one is associated with a particular
 * interface command.  For example, entering the "h" command into a serial console (like TeraTerm or
 * Minicom) and hitting Enter will cause the PrintHelp_CMD() function to be called by the state
 * machine above.
 */

/* Compute the 16-bit crc of the loaded application.
 * Syntax: crc
 * Response: uint16_t
 */
static void CalcAppCRC_CMD(const char *cmdbuf, uint8_t len)
{
	union
	{
		uint16_t val;
		uint8_t  by[2];
	} crc;

	crc.val = CalculateAppCRC();

	UART_TxHexByte(crc.by[1]);
	UART_TxHexByte(crc.by[0]);
	UART_TxChar('\r');
}

/* Print help info for all registered commands.
 * Syntax: h
 * Response: multiline string (printed in state machine)
 */
static void PrintHelp_CMD(const char *cmdbuf, uint8_t len)
{
	m_cmdstate = eCmd_Help;
}

/* Show the version string for the app/bootloader.  This is defined in appcfg.h, which is where
 * application-specific macros are defined.
 * Syntax: v
 * Response: string
 */
static void ShowVersion_CMD(const char *cmdbuf, uint8_t len)
{
	UART_TxString_P(m_versionstr);
}
