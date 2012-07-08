/* cmd.h
 *
 * Command interface for talking over the serial port to the application or bootloader.  Common
 * commands are defined in here along with a function allowing you to add your own.
 */

#ifndef INCLUDE_CMD_H
#define INCLUDE_CMD_H

#include "appcfg.h"
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdbool.h>

// size must be 255 or less
#ifndef CMD_BUFSIZE
#  define CMD_BUFSIZE  32
#endif

#ifndef CMD_MAXCMDS
#  define CMD_MAXCMDS 10
#endif

// takes a pointer to the command buffer and length of the string
typedef void (*cmdhandler_t)(const char *, uint8_t);

typedef struct cmdinfo_tag
{
	const char *name;
	cmdhandler_t cmdfunc;
	const prog_char *help;    // must be non-NULL (use "" to not display help)
} cmdinfo_t;

void Cmd_InitInterface();
bool Cmd_RegisterCommand(const char *cmdname, cmdhandler_t cmdfunc, const prog_char *help);
void Cmd_ProcessInterface();

#endif // INCLUDE_CMD_H
