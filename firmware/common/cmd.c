/* cmd.c
 *
 * Command interface for talking over the serial port.  Common commands are defined in here, but any
 * projects using this file can also define their own.
 */

#include "uart.h"
#include "cmd.h"
#include <string.h>

static cmdinfo_t m_cmds[CMD_MAXCMDS];
static uint8_t m_numcmds;

static char m_cmdbuf[CMD_BUFSIZE+1];    // one extra for null terminator
static uint8_t m_cmdcount;

static enum cmdstate_t
{
	eCmd_Prompt = 0,
	eCmd_Receive,
	eCmd_Run,
	eCmd_Reset
} m_cmdstate;


void Cmd_InitInterface()
{
	m_cmdstate = eCmd_Prompt;
	m_numcmds = 0;
	m_cmdcount = 0;

	UART_TxString_P(PSTR("\r\r" VERSION_STRING));
}

void Cmd_AddCommand(const prog_char *name, 	cmdhandler_t cmdfunc, const prog_char *help)
{
	if(m_numcmds < CMD_MAXCMDS)
	{
		m_cmds[m_numcmds].name = name;
		m_cmds[m_numcmds].cmdfunc = cmdfunc;
		m_cmds[m_numcmds].help = help;
		++m_numcmds;
	}
	else
		UART_TxString_P("Too many commands!\r");
}
