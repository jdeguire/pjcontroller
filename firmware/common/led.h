/* led.h
 * 
 * Contains macros mapping LED colors to the pins on the device to which they are connected.
 */

#ifndef INCLUDE_LED_H_
#define INCLUDE_LED_H_

#include "regmap.h"

#define GREEN_LED_DDR  DDRDbits.ddd7
#define GREEN_LED_PORT PORTDbits.pd7
#define GREEN_LED_PIN  PINDbits.pind7

#define AMBER_LED_DDR  DDRDbits.ddd6
#define AMBER_LED_PORT PORTDbits.pd6
#define AMBER_LED_PIN  PINDbits.pind6

#define RED_LED_DDR  DDRDbits.ddd5
#define RED_LED_PORT PORTDbits.pd5
#define RED_LED_PIN  PINDbits.pind5

#endif // INCLUDE_LED_H_
