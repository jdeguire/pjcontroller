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
 * File:   localcmd.h
 * Author: Jesse DeGuire
 *
 * Serial interface commmands specific to the bootloader.  See common/cmd.c for the implementation
 * of the interface itself.
 */

#ifndef INCLUDE_LOCALCMD_H_
#define INCLUDE_LOCALCMD_H_

#define APP_CHECKSUM_VALID 0xAA55

void RegisterBootloaderCommands();

#endif // INCLUDE_LOCALCMD_H_
