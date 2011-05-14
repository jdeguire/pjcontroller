/* main.c
 *
 * Main module for the PJC controller application.
 */

#include "regmap.h"

int main(void)
{
	DDRDbits.ddd7 = 1;   // output
	PORTDbits.pd7 = 1;   // turn on

	while(1);
}
