/* cmd.h
 *
 * Command interface for talking over the serial port.  Common commands are defined in here, but any
 * projects using this file can also define their own.
 */

#ifndef INCLUDE_CMD_H
#define INCLUDE_CMD_H

#include "system.h"
#include <avr/pgmspace.h>
#include <stdint.h>

#ifndef CMD_BUFSIZE
#  define CMD_BUFSIZE  32
#endif

#ifndef CMD_MAXCMDS
#  define CMD_MAXCMDS  10
#endif

#ifndef VERSION_STRING
#  define VERSION_STRING ""
#endif

typedef void (*cmdhandler_t)(const char *, uint8_t);

typedef struct cmdinfo_tag
{
	const prog_char *name;
	cmdhandler_t cmdfunc;
	const prog_char *help;
} cmdinfo_t;

#endif // INCLUDE_CMD_H
