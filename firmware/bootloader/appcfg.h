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
 * File:   appcfg.h
 * Author: Jesse DeGuire
 *
 * Project-specific defines and settings used in multiple places.  This file is included in several
 * common code modules as well, allowing you to change their settings.  For example, you could
 * define the UART_RX_BUFSIZE macro to change the size of the UART receive buffer.
 */

#ifndef INCLUDE_SYSTEM_H_
#define INCLUDE_SYSTEM_H_

#include "misc.h"

#define PJC_BOOTLOADER__

#define CMD_BUFSIZE 64
#define CMD_MAXCMDS 13

#define FIRMWARE_VERSION 0
#define VERSION_STRING "PJC Bootloader v"STRINGIFY(FIRMWARE_VERSION)


#endif // INCLUDE_SYSTEM_H_
