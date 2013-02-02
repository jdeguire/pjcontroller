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
 * File:   cmd.h
 * Author: Jesse DeGuire
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
	const __flash char *help;    // must be non-NULL (use "" to not display help)
} cmdinfo_t;

void Cmd_InitInterface();
bool Cmd_RegisterCommand(const char *cmdname, cmdhandler_t cmdfunc, const __flash char *help);
void Cmd_ProcessInterface();

#endif // INCLUDE_CMD_H
