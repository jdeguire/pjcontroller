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
 * File:   deviceprog.h
 * Author: Jesse DeGuire
 *
 * Routines for dealing with the onboard flash and EEPROM memory.
 */

#ifndef INCLUDE_DEVICEPROG_H_
#define INCLUDE_DEVICEPROG_H_

#include "sharedmem.h"
#include <stdint.h>
#include <stdbool.h>

void Flash_WriteLatch(uint16_t offset, uint16_t insword);
void Flash_ProgramPage(uint16_t addr);
bool Flash_VerifyPage(uint16_t page, uint16_t *buf);
void Flash_EraseApp();
void EEPROM_EraseData();

#endif // INCLUDE_DEVICEPROG_H_
