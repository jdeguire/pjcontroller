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
 * Contains macros mapping LED colors to the pins on the device to which they are connected.
 */

#ifndef INCLUDE_LED_H_
#define INCLUDE_LED_H_

#include "regmap.h"

#define GREEN_LED_DDR  DDRDbits.ddd7
#define GREEN_LED_PORT PORTDbits.pd7
#define GREEN_LED_PIN  PINDbits.pind7

#define YELLOW_LED_DDR  DDRDbits.ddd6
#define YELLOW_LED_PORT PORTDbits.pd6
#define YELLOW_LED_PIN  PINDbits.pind6

#define RED_LED_DDR  DDRDbits.ddd5
#define RED_LED_PORT PORTDbits.pd5
#define RED_LED_PIN  PINDbits.pind5

#endif // INCLUDE_LED_H_
