/* bootcmd.h
 *
 * Command interface for talking over the serial port to the bootloader.  This is a trimmed-down
 * version of cmd.h found in the common/ directory.
 */


#ifndef INCLUDE_BOOTCMD_H
#define INCLUDE_BOOTCMD_H

#include "system.h"
#include <stdint.h>

#ifndef CMD_BUFSIZE
#  define CMD_BUFSIZE  32
#endif

#ifndef MAX_CMDS
#  define MAX_CMDS 10
#endif

typedef void (*cmdhandler_t)(const char *, uint8_t);

typedef struct cmdinfo_tag
{
	const char *name;
	cmdhandler_t cmdfunc;
} cmdinfo_t;

void Cmd_InitInterface();
void Cmd_RegisterCommand(const char *cmdname, cmdhandler_t cmdfunc);
void Cmd_ProcessInterface();

#endif // INCLUDE_BOOTCMD_H
