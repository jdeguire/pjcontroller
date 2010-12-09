/* bootcmd.c
 *
 * Command interface for talking over the serial port to the bootloader.  This is a trimmed-down
 * version of cmd.c found in the common/ directory.
 */

#include "uart.h"
#include "cmd.h"
#include <string.h>

static void ShowVersion_CMD(const char *cmdbuf, uint8_t count);
static void HelloTest_CMD(const char *cmdbuf, uint8_t count);

static cmdinfo_t m_cmds[] = {{"v", ShowVersion_CMD},
							 {"hello", HelloTest_CMD}};

static uint8_t m_numcmds = sizeof(m_cmds) / sizeof(cmdinfo_t);

static char m_cmdbuf[CMD_BUFSIZE+1];    // one extra for null terminator
static uint8_t m_cmdcount;

static enum cmdstate_t
{
	eCmd_Prompt = 0,
	eCmd_Receive,
	eCmd_Run,
} m_cmdstate;


void Cmd_InitInterface()
{
	m_cmdstate = eCmd_Prompt;
	UART_TxString("\r\r" VERSION_STRING);
}

static void ShowVersion_CMD(const char *cmdbuf, uint8_t count)
{
	UART_TxString(VERSION_STRING);
}

static void HelloTest_CMD(const char *cmdbuf, uint8_t count)
{
	UART_TxString("Hello, world and ");
	UART_TxString(cmdbuf+strlen("hello "));
}

void Cmd_ProcessInterface()
{
	switch(m_cmdstate)
	{
		case eCmd_Prompt:
			UART_TxString("\r\r#> ");
			memset(m_cmdbuf, 0, CMD_BUFSIZE+1);
			m_cmdcount = 0;
			++m_cmdstate;
			break;
		case eCmd_Receive:
			if(UART_RxAvailable() > 0)
			{
				char c = UART_RxChar();
				
				m_cmdbuf[m_cmdcount] = c;
				++m_cmdcount;
				
				// either got a command or buffer is full
				if(c == '\r'  ||  m_cmdcount >= CMD_BUFSIZE)
					++m_cmdstate;
			}
			break;
		case eCmd_Run:
			if(m_numcmds > 0)
			{
				uint8_t i;

				// look for the right command and call its function
				for(i = 0; i < m_numcmds; ++i)
				{
					size_t cmdlen = strlen(m_cmds[i].name);

					if(0 == strncmp(m_cmds[i].name, m_cmdbuf, cmdlen)  &&
					   (m_cmdbuf[cmdlen] == ' '  ||  m_cmdbuf[cmdlen] == '\r'))
					{
						m_cmds[i].cmdfunc(m_cmdbuf, m_cmdcount);
						break;
					}
				}
				
				// didn't find the command
				if(i == m_numcmds)
					UART_TxString("Unknown command");
			}
			m_cmdstate = eCmd_Prompt;
			break;
		default:
			break;
	}
}
