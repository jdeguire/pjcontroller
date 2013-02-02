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
 * File:   sharedmem.h
 * Author: Jesse DeGuire
 *
 * Used for routines and memory shared between the PJC Bootloader and the application.
 */

#ifndef INCLUDE_SHAREDMEM_H_
#define INCLUDE_SHAREDMEM_H_

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

// These are defined in the custom linker scripts for the app and bootloader.
// Their addresses are the important part.
extern const __flash uint8_t _app_space_start;
extern const __flash uint8_t _app_space_end;

#define APP_SPACE_START ((uint16_t)&_app_space_start)
#define APP_SPACE_END   ((uint16_t)&_app_space_end)

#define NUM_APP_PAGES   ((APP_SPACE_END + (SPM_PAGESIZE - 1)) / SPM_PAGESIZE)

bool AppRestartRequested();
void ClearAppRestartRequest();
void RestartApp() __attribute__((noreturn));
void RestartBootloader() __attribute__((noreturn));
uint16_t CalculateAppCRC();

#endif // INCLUDE_SHAREDMEM_H_
