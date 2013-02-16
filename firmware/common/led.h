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
 * File:   led.h
 * Author: Jesse DeGuire
 * 
 * Contains macros for setting and inverting the LEDs connected to the microcontroller.
 */

#ifndef INCLUDE_LED_H_
#define INCLUDE_LED_H_

#include "regmap.h"

// set LED pins as outputs and disable them
#define InitLEDs() do{DDRD |= 0xE0; PORTD &= 0x1F;} while(0)

#define SetGreenLED() (PORTDbits.pd7  = 1)
#define ClrGreenLED() (PORTDbits.pd7  = 0)
#define InvGreenLED() (PINDbits.pind7 = 1)
#define GetGreenLED() (PORTDbits.pd7)

#define SetYellowLED() (PORTDbits.pd6  = 1)
#define ClrYellowLED() (PORTDbits.pd6  = 0)
#define InvYellowLED() (PINDbits.pind6 = 1)
#define GetYellowLED() (PORTDbits.pd6)

#define SetRedLED() (PORTDbits.pd5  = 1)
#define ClrRedLED() (PORTDbits.pd5  = 0)
#define InvRedLED() (PINDbits.pind5 = 1)
#define GetRedLED() (PORTDbits.pd5)

#endif // INCLUDE_LED_H_
