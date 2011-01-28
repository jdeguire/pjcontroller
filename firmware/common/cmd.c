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


static void CalcAppCRC_CMD(const char *cmdbuf, uint8_t count);
static void PrintHelp_CMD(const char *cmdbuf, uint8_t count);
static void JumpToOther_CMD(const char *cmdbuf, uint8_t count) __attribute__((noreturn));
static void ResetDevice_CMD(const char *cmdbuf, uint8_t count) __attribute__((noreturn));
static void ShowVersion_CMD(const char *cmdbuf, uint8_t count);

static char m_cmdbuf[CMD_BUFSIZE+1];    // one extra for null terminator
static uint8_t m_cmdcount;

static enum cmdstate_t
{
	eCmd_Prompt = 0,
	eCmd_Receive,
	eCmd_Run,
} m_cmdstate;

// attributes cannot be used with constants, so we have to define these separately
static const prog_char m_crchelp[]   = "Calculate application crc";
static const prog_char m_helphelp[]  = "Print this help";
static const prog_char m_verhelp[]   = "Show version info";

#ifdef __PJC_BOOTLOADER__
static const prog_char m_jumphelp[]  = "Force app start";
static const prog_char m_resethelp[] = "Restart bootloader";
#else
static const prog_char m_jumphelp[]  = "Jump to bootloader";
static const prog_char m_resethelp[] = "Restart app";
#endif

static cmdinfo_t m_cmds[MAX_CMDS] = {{"crc", CalcAppCRC_CMD, m_crchelp},
									 {"h", PrintHelp_CMD, m_helphelp},
									 {"j", JumpToOther_CMD, m_jumphelp},
									 {"r", ResetDevice_CMD, m_resethelp},
									 {"v", ShowVersion_CMD, m_verhelp}};
static uint8_t m_numcmds = 5;


/* Initialize the interface by printing out version info and setting up the interface state machine.
 */
void Cmd_InitInterface()
{
	m_cmdstate = eCmd_Prompt;
	UART_TxString("\r\r" VERSION_STRING "\r");
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
 			UART_TxString("\r#> ");
			memset(m_cmdbuf, 0, CMD_BUFSIZE+1);
			m_cmdcount = 0;
			++m_cmdstate;
			break;
		case eCmd_Receive:
			if(UART_RxAvailable() > 0)
			{
				char c = UART_RxChar();

				if(0x08 == c)           // backspace -- remove last character
				{
					if(m_cmdcount > 0)
						m_cmdbuf[--m_cmdcount] = '\0';
				}
				else if(0x1B == c)      // escape -- clear prompt
				{
					m_cmdstate = eCmd_Prompt;
				}
				else                    // add the character to the buffer
				{
					m_cmdbuf[m_cmdcount] = c;
					++m_cmdcount;

					// either got a command or buffer is full
					if(c == '\r'  ||  m_cmdcount >= CMD_BUFSIZE)
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
					UART_TxString("Unknown command\r");
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

static void CalcAppCRC_CMD(const char *cmdbuf, uint8_t count)
{
	char crcstr[5];

	utoa(CalculateAppCRC(), crcstr, 16);
	UART_TxString("App CRC: 0x");
	UART_TxString(crcstr);
	UART_TxChar('\r');
}

static void PrintHelp_CMD(const char *cmdbuf, uint8_t count)
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

#ifdef __PJC_BOOTLOADER__

static void JumpToOther_CMD(const char *cmdbuf, uint8_t count)
{
	// This allows the user the ability to force the app to start, even if the bootloader says
	// there's a problem.  This can be useful for testing.
	asm volatile("jmp 0x0");
	for(;;) ;
}

static void ResetDevice_CMD(const char *cmdbuf, uint8_t count)
{
	RestartBootloader();
}

#else

static void JumpToOther_CMD(const char *cmdbuf, uint8_t count)
{
	RestartBootloader();
}

static void ResetDevice_CMD(const char *cmdbuf, uint8_t count)
{
	RestartApp();
}

#endif  // __PJC_BOOTLOADER__

static void ShowVersion_CMD(const char *cmdbuf, uint8_t count)
{
	UART_TxString(VERSION_STRING);
}
