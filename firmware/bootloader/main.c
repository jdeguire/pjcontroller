#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
	// LED on PC5
	DDRC |= (1 << DD5);       // 1 sets pin to output
	PORTC |= (1 << PC5);     // start with output low

	while(1)
	{
		PINC |= (1 << PINC5); // set this to toggle PORTxn bit; read to get pin state
		_delay_ms(1000);
	}
}
