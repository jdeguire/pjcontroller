/* cmd.h
 *
 * Command interface for talking over the serial port to the application or bootloader.  Common
 * commands are defined in here along with a function allowing you to add your own.
 */

#include "uart.h"
#include "sharedmem.h"
#include "cmd.h"
#include <string.h>
#include <stdlib.h>


static void CalcAppCRC_CMD(const char *cmdbuf, uint8_t len);
static void PrintHelp_CMD(const char *cmdbuf, uint8_t len);
static void ShowVersion_CMD(const char *cmdbuf, uint8_t len)  __attribute__((noinline));

static const prog_char m_versionstr[] = "\r\r" VERSION_STRING "\r";


static char m_cmdbuf[CMD_BUFSIZE+1];    // one extra for null terminator
static uint8_t m_cmdlen;

static enum cmdstate_t
{
	eCmd_Prompt = 0,
	eCmd_Receive,
	eCmd_Run,
} m_cmdstate;

// attributes cannot be used with constants, so we have to define these separately
static const prog_char m_crchelp[]   = "Calculate app crc";
static const prog_char m_helphelp[]  = "Print help";
static const prog_char m_verhelp[]   = "Show version";

static cmdinfo_t m_cmds[MAX_CMDS] = {{"crc", CalcAppCRC_CMD, m_crchelp},
									 {"h", PrintHelp_CMD, m_helphelp},
									 {"v", ShowVersion_CMD, m_verhelp}};
static uint8_t m_numcmds = 3;


/* Initialize the interface by printing out version info and setting up the interface state machine.
 */
void Cmd_InitInterface()
{
	m_cmdstate = eCmd_Prompt;
	UART_TxString_P(m_versionstr);
}

/* Add a new interface command that can be called via the connected serial console.  Parameters are
 * the seial command, the function that the command will call, and a short help string for the
 * command.  The maximum number of commands is deteremined by the MAX_CMDS macro.  This does not
 * check for duplicate command names.
 *
 * Return True if a new command was added and False if it couldn't be added.
 */
bool Cmd_RegisterCommand(const char *cmdname, cmdhandler_t cmdfunc, const prog_char *help)
{
	bool result = false;

	if(m_numcmds < MAX_CMDS)
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
 			UART_TxData("\r#> ", 4);
			m_cmdlen = 0;
			memset(m_cmdbuf, 0, sizeof(m_cmdbuf));
			++m_cmdstate;
			break;
		case eCmd_Receive:
			if(UART_RxAvailable() > 0)
			{
				char c = UART_RxChar();

				if(0x08 == c)           // backspace -- remove last character
				{
					if(m_cmdlen > 0)
						m_cmdbuf[--m_cmdlen] = '\0';
				}
				else if(0x1B == c)      // escape -- clear prompt
				{
					m_cmdstate = eCmd_Prompt;
				}
				else                    // add the character to the buffer
				{
					m_cmdbuf[m_cmdlen] = c;
					++m_cmdlen;

					// either got a command or buffer is full
					if(c == '\r'  ||  m_cmdlen >= CMD_BUFSIZE)
						++m_cmdstate;
				}
			}
			break;
		case eCmd_Run:
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
			m_cmdstate = eCmd_Prompt;
			break;
		default:
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

/* Print help info for all registered commands.  If the help string is NULL for that command, then
 * only the command name is printed.
 * Syntax: h
 */
static void PrintHelp_CMD(const char *cmdbuf, uint8_t len)
{
	int i;
	
	for(i = 0; i < m_numcmds; ++i)
	{
		while(UART_TxAvailable() < strlen(m_cmds[i].name) + 1)
			;

		UART_TxString(m_cmds[i].name);

		if(m_cmds[i].help != NULL)
		{
			while(UART_TxAvailable() < (strlen_P(m_cmds[i].help) + 4))
				;

			UART_TxData(" - ", 3);
			UART_TxString_P(m_cmds[i].help);
		}
		
		UART_TxChar('\r');
	}

	while(UART_TxAvailable() < 5)
		;
}

/* Show the version string for the app/bootloader.  This is defined in system.h, which is where
 * application-specific macros are defined.
 * Syntax: v
 */
static void ShowVersion_CMD(const char *cmdbuf, uint8_t len)
{
	UART_TxString_P(m_versionstr + 2);
}
