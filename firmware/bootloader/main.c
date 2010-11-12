#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

struct PORTCbits_t
{
	uint8_t pc0:1;
	uint8_t pc1:1;
	uint8_t pc2:1; 
	uint8_t pc3:1;
	uint8_t pc4:1; 
	uint8_t pc5:1;
	uint8_t pc6:1; 
	uint8_t pc7:1;
};

struct PINCbits_t
{
	uint8_t pinc0:1; 
	uint8_t pinc1:1;
	uint8_t pinc2:1; 
	uint8_t pinc3:1;
	uint8_t pinc4:1; 
	uint8_t pinc5:1;
	uint8_t pinc6:1; 
	uint8_t pinc7:1;
};

#define PORTCbits (*(struct PORTCbits_t *)_SFR_MEM_ADDR(PORTC))
#define PINCbits (*(struct PINCbits_t *)_SFR_MEM_ADDR(PINC))

int main(void)
{
	// LED on PC5
	DDRC |= (1 << DD5);       // 1 sets pin to output
	PORTCbits.pc5 = 0;        // start with output low

	while(1)
	{
		PINCbits.pinc5 = 1;   // toggle state of output pin
		_delay_ms(1000);
	}
}
