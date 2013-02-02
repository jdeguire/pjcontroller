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
 * File:   regmap.h
 * Author: Jesse DeGuire
 * 
 * A register map for the ATmega328P (and the related 168/88/48/etc.) allowing access to the
 * device's special function registers as though they were structures.	The addresses are known at
 * compile time, meaning that the compiler can use the faster SBI/CBI/IN/OUT instructions as
 * appropriate.
 */

#ifndef INCLUDE_REGMAP_H_
#define INCLUDE_REGMAP_H_

#include <avr/io.h>

struct PINBbits_t
{
	uint8_t pinb0:1; 
	uint8_t pinb1:1;
	uint8_t pinb2:1; 
	uint8_t pinb3:1;
	uint8_t pinb4:1; 
	uint8_t pinb5:1;
	uint8_t pinb6:1; 
	uint8_t pinb7:1;
};

#define PINBbits (*(volatile struct PINBbits_t *)_SFR_MEM_ADDR(PINB))


struct DDRBbits_t
{
	uint8_t ddb0:1;
	uint8_t ddb1:1;
	uint8_t ddb2:1;
	uint8_t ddb3:1;
	uint8_t ddb4:1;
	uint8_t ddb5:1;
	uint8_t ddb6:1;
	uint8_t ddb7:1;
};

#define DDRBbits (*(volatile struct DDRBbits_t *)_SFR_MEM_ADDR(DDRB))


struct PORTBbits_t
{
	uint8_t pb0:1;
	uint8_t pb1:1;
	uint8_t pb2:1; 
	uint8_t pb3:1;
	uint8_t pb4:1; 
	uint8_t pb5:1;
	uint8_t pb6:1; 
	uint8_t pb7:1;
};

#define PORTBbits (*(volatile struct PORTBbits_t *)_SFR_MEM_ADDR(PORTB))


struct PINCbits_t
{
	uint8_t pinc0:1; 
	uint8_t pinc1:1;
	uint8_t pinc2:1; 
	uint8_t pinc3:1;
	uint8_t pinc4:1; 
	uint8_t pinc5:1;
	uint8_t pinc6:1; 
	uint8_t		 :1;
};

#define PINCbits (*(volatile struct PINCbits_t *)_SFR_MEM_ADDR(PINC))


struct DDRCbits_t
{
	uint8_t ddc0:1;
	uint8_t ddc1:1;
	uint8_t ddc2:1;
	uint8_t ddc3:1;
	uint8_t ddc4:1;
	uint8_t ddc5:1;
	uint8_t ddc6:1;
	uint8_t		:1;
};

#define DDRBbits (*(volatile struct DDRBbits_t *)_SFR_MEM_ADDR(DDRB))


struct PORTCbits_t
{
	uint8_t pc0:1;
	uint8_t pc1:1;
	uint8_t pc2:1; 
	uint8_t pc3:1;
	uint8_t pc4:1; 
	uint8_t pc5:1;
	uint8_t pc6:1; 
	uint8_t	   :1;
};

#define PORTCbits (*(volatile struct PORTCbits_t *)_SFR_MEM_ADDR(PORTC))


struct PINDbits_t
{
	uint8_t pind0:1; 
	uint8_t pind1:1;
	uint8_t pind2:1; 
	uint8_t pind3:1;
	uint8_t pind4:1; 
	uint8_t pind5:1;
	uint8_t pind6:1; 
	uint8_t pind7:1;
};

#define PINDbits (*(volatile struct PINDbits_t *)_SFR_MEM_ADDR(PIND))


struct DDRDbits_t
{
	uint8_t ddd0:1;
	uint8_t ddd1:1;
	uint8_t ddd2:1;
	uint8_t ddd3:1;
	uint8_t ddd4:1;
	uint8_t ddd5:1;
	uint8_t ddd6:1;
	uint8_t ddd7:1;
};

#define DDRDbits (*(volatile struct DDRDbits_t *)_SFR_MEM_ADDR(DDRD))


struct PORTDbits_t
{
	uint8_t pd0:1;
	uint8_t pd1:1;
	uint8_t pd2:1; 
	uint8_t pd3:1;
	uint8_t pd4:1; 
	uint8_t pd5:1;
	uint8_t pd6:1; 
	uint8_t pd7:1;
};

#define PORTDbits (*(volatile struct PORTDbits_t *)_SFR_MEM_ADDR(PORTD))


struct TIFR0bits_t
{
	uint8_t	 tov0:1;
	uint8_t ocf0a:1;
	uint8_t ocf0b:1;
	uint8_t		 :5;
};

#define TIFR0bits (*(volatile struct TIFR0bits_t *)_SFR_MEM_ADDR(TIFR0))


struct TIFR1bits_t
{
	uint8_t	 tov1:1;
	uint8_t ocf1a:1;
	uint8_t ocf1b:1;
	uint8_t		 :2;
	uint8_t	 icf1:1;
	uint8_t		 :2;
};

#define TIFR1bits (*(volatile struct TIFR1bits_t *)_SFR_MEM_ADDR(TIFR1))


struct TIFR2bits_t
{
	uint8_t	 tov2:1;
	uint8_t ocf2a:1;
	uint8_t ocf2b:1;
	uint8_t		 :5;
};

#define TIFR2bits (*(volatile struct TIFR2bits_t *)_SFR_MEM_ADDR(TIFR2))


struct PCIFRbits_t
{
	uint8_t pcif0:1;
	uint8_t pcif1:1;
	uint8_t pcif2:1;
	uint8_t		 :5;
};

#define PCIFRbits (*(volatile struct PCIFRbits_t *)_SFR_MEM_ADDR(PCIFR))


struct EIFRbits_t
{
	uint8_t intf0:1;
	uint8_t intf1:1;
	uint8_t		 :6;
};

#define EIFRbits (*(volatile struct EIFRbits_t *)_SFR_MEM_ADDR(EIFR))


struct EIMSKbits_t
{
	uint8_t int0:1;
	uint8_t int1:1;
	uint8_t		:6;
};

#define EIMSKbits (*(volatile struct EIMSKbits_t *)_SFR_MEM_ADDR(EIMSK))


struct GPIORXbits_t
{
	uint8_t gpio0:1;
	uint8_t gpio1:1;
	uint8_t gpio2:1;
	uint8_t gpio3:1;
	uint8_t gpio4:1;
	uint8_t gpio5:1;
	uint8_t gpio6:1;
	uint8_t gpio7:1;
};

#define GPIOR0bits (*(volatile struct GPIORXbits_t *)_SFR_MEM_ADDR(GPIOR0))
#define GPIOR1bits (*(volatile struct GPIORXbits_t *)_SFR_MEM_ADDR(GPIOR1))
#define GPIOR2bits (*(volatile struct GPIORXbits_t *)_SFR_MEM_ADDR(GPIOR2))


union EECRbits_t
{
	struct
	{
		uint8_t	 eere:1;
		uint8_t	 eepe:1;
		uint8_t eempe:1;
		uint8_t eerie:1;
		uint8_t eepm0:1;
		uint8_t eepm1:1;
		uint8_t		 :2;
	};
	struct
	{
		uint8_t		:4;
		uint8_t eepm:2;
		uint8_t		:2;
	};
};

#define EECRbits (*(volatile union EECRbits_t *)_SFR_MEM_ADDR(EECR))


// EEDR (EEPROM Data Register) does not have any fields
// EEARL (EEPROM Address Register Low) does not have any fields
// EEARH (EEPROM Address Register High) does not have any fields


struct GTCCRbits_t
{
	uint8_t psrsync:1;
	uint8_t	 psrasy:1;
	uint8_t		   :5;
	uint8_t		tsm:1;
};

#define GTCCRbits (*(volatile struct GTCCRbits_t *)_SFR_MEM_ADDR(GTCCR))


union TCCR0Abits_t
{
	struct
	{
		uint8_t	 wgm00:1;
		uint8_t	 wgm01:1;
		uint8_t		  :2;
		uint8_t com0b0:1;
		uint8_t com0b1:1;
		uint8_t com0a0:1;
		uint8_t com0a1:1;
	};
	struct
	{
		uint8_t	 wgm0:2;
		uint8_t		 :2;
		uint8_t com0b:2;
		uint8_t com0a:2;
	};
};

#define TCCR0Abits (*(volatile union TCCR0Abits_t *)_SFR_MEM_ADDR(TCCR0A))


union TCCR0Bbits_t
{
	struct
	{
		uint8_t	 cs00:1;
		uint8_t	 cs01:1;
		uint8_t	 cs02:1;
		uint8_t wgm02:1;
		uint8_t		 :2;
		uint8_t foc0b:1;
		uint8_t foc0a:1;
	};
	struct
	{
		uint8_t cs0:3;
		uint8_t	   :5;
	};
};

#define TCCR0Bbits (*(volatile union TCCR0Bbits_t *)_SFR_MEM_ADDR(TCCR0B))


// TCNT0 (Timer 0 Counter) does not have any fields
// OCR0A (Timer 0 Output Compare Register A) does not have any fields
// OCR0B (Timer 0 Output Compare Register B) does not have any fields


union SPCRbits_t
{
	struct
	{
		uint8_t spr0:1;
		uint8_t spr1:1;
		uint8_t cpha:1;
		uint8_t cpol:1;
		uint8_t mstr:1;
		uint8_t dord:1;
		uint8_t	 spe:1;
		uint8_t spie:1;
	};
	struct
	{
		uint8_t spr:2;
		uint8_t	   :6;
	};
};

#define SPCRbits (*(volatile union SPCRbits_t *)_SFR_MEM_ADDR(SPCR))


struct SPSRbits_t
{
	uint8_t spi2x:1;
	uint8_t		 :5;
	uint8_t	 wcol:1;
	uint8_t	 spif:1;
};

#define SPSRbits (*(volatile struct SPSRbits_t *)_SFR_MEM_ADDR(SPSR))


// SPDR (SPI Data Register) does not have any fields


union ACSRbits_t
{
	struct
	{
		uint8_t acis0:1;
		uint8_t acis1:1;
		uint8_t	 acic:1;
		uint8_t	 acie:1;
		uint8_t	  aci:1;
		uint8_t	  aco:1;
		uint8_t	 acbg:1;
		uint8_t	  acd:1;
	};
	struct
	{
		uint8_t acis:2;
		uint8_t		:6;
	};
};

#define ACSRbits (*(volatile union ACSRbits_t *)_SFR_MEM_ADDR(ACSR))


union SMCRbits_t
{
	struct
	{
		uint8_t	 se:1;
		uint8_t sm0:1;
		uint8_t sm1:1;
		uint8_t sm2:1;
		uint8_t	   :4;
	};
	struct
	{
		uint8_t	  :1;
		uint8_t sm:3;
		uint8_t	  :4;
	};
};

#define SMCRbits (*(volatile union SMCRbits_t *)_SFR_MEM_ADDR(SMCR))


struct MCUSRbits_t
{
	uint8_t	 porf:1;
	uint8_t extrf:1;
	uint8_t	 borf:1;
	uint8_t	 wdrf:1;
	uint8_t		 :4;
};

#define MCUSRbits (*(volatile struct MCUSRbits_t *)_SFR_MEM_ADDR(MCUSR))


struct MCUCRbits_t
{
	uint8_t	 ivce:1;
	uint8_t ivsel:1;
	uint8_t		 :2;
	uint8_t	  pud:1;
	uint8_t bodse:1;
	uint8_t	 bods:1;
	uint8_t		 :1;
};

#define MCUCRbits (*(volatile struct MCUCRbits_t *)_SFR_MEM_ADDR(MCUCR))


struct SPMCSRbits_t
{
	uint8_t selfprgen:1;
	uint8_t		pgers:1;
	uint8_t		pgwrt:1;
	uint8_t	   blbset:1;
	uint8_t	   rwwsre:1;
	uint8_t			 :1;
	uint8_t		rwwsb:1;
	uint8_t		spmie:1;
};

#define SPMCSRbits (*(volatile struct SPMCSRbits_t *)_SFR_MEM_ADDR(SPMCSR))


struct SPLbits_t
{
	uint8_t sp0:1;
	uint8_t sp1:1;
	uint8_t sp2:1;
	uint8_t sp3:1;
	uint8_t sp4:1;
	uint8_t sp5:1;
	uint8_t sp6:1;
	uint8_t sp7:1;
};

#define SPLbits (*(volatile struct SPLbits_t *)_SFR_MEM_ADDR(SPL))

struct SPHbits_t
{
	uint8_t	 sp8:1;
	uint8_t	 sp9:1;
	uint8_t sp10:1;
	uint8_t		:5;
};

#define SPHbits (*(volatile struct SPHbits_t *)_SFR_MEM_ADDR(SPH))


struct SREGbits_t
{
	uint8_t c:1;
	uint8_t z:1;
	uint8_t n:1;
	uint8_t v:1;
	uint8_t s:1;
	uint8_t h:1;
	uint8_t t:1;
	uint8_t i:1;
};

#define SREGbits (*(volatile struct SREGbits_t *)_SFR_MEM_ADDR(SREG))


union WDTCSRbits_t
{
	struct
	{
		uint8_t wdp0:1;
		uint8_t wdp1:1;
		uint8_t wdp2:1;
		uint8_t	 wde:1;
		uint8_t wdce:1;
		uint8_t wdp3:1;
		uint8_t wdie:1;
		uint8_t wdif:1;
	};
	struct
	{
		uint8_t wdp:3;
		uint8_t	   :5;
	};
};

#define WDTCSRbits (*(volatile union WDTCSRbits_t *)_SFR_MEM_ADDR(WDTCSR))


union CLKPRbits_t
{
	struct
	{
		uint8_t clkps0:1;
		uint8_t clkps1:1;
		uint8_t clkps2:1;
		uint8_t clkps3:1;
		uint8_t		  :3;
		uint8_t clkpce:1;
	};
	struct
	{
		uint8_t clkps:4;
		uint8_t		 :4;
	};
};

#define CLKPRbits (*(volatile union CLKPRbits_t *)_SFR_MEM_ADDR(CLKPR))


struct PRRbits_t
{
	uint8_t	   pradc:1;
	uint8_t prusart0:1;
	uint8_t	   prspi:1;
	uint8_t	  prtim1:1;
	uint8_t			:1;
	uint8_t	  prtim0:1;
	uint8_t	  prtim2:1;
	uint8_t	   prtwi:1;
};

#define PRRbits (*(volatile struct PRRbits_t *)_SFR_MEM_ADDR(PRR))


// OSCCAL (Oscillator Calibration Register) does not have any fields


struct PCICRbits_t
{
	uint8_t pcie0:1;
	uint8_t pcie1:1;
	uint8_t pcie2:1;
	uint8_t		 :5;
};

#define PCICRbits (*(volatile struct PCICRbits_t *)_SFR_MEM_ADDR(PCICR))


union EICRAbits_t
{
	struct
	{
		uint8_t isc00:1;
		uint8_t isc01:1;
		uint8_t isc10:1;
		uint8_t isc11:1;
		uint8_t		 :4;
	};
	struct
	{
		uint8_t isc0:2;
		uint8_t isc1:2;
		uint8_t		:4;
	};
};

#define EICRAbits (*(volatile union EICRAbits_t *)_SFR_MEM_ADDR(EICRA))


struct PCMSK0bits_t
{
	uint8_t pcint0:1;
	uint8_t pcint1:1;
	uint8_t pcint2:1;
	uint8_t pcint3:1;
	uint8_t pcint4:1;
	uint8_t pcint5:1;
	uint8_t pcint6:1;
	uint8_t pcint7:1;
};

#define PCMSK0bits (*(volatile struct PCMSK0bits_t *)_SFR_MEM_ADDR(PCMSK0))


struct PCMSK1bits_t
{
	uint8_t	 pcint8:1;
	uint8_t	 pcint9:1;
	uint8_t pcint10:1;
	uint8_t pcint11:1;
	uint8_t pcint12:1;
	uint8_t pcint13:1;
	uint8_t pcint14:1;
	// there is no 15...
};

#define PCMSK1bits (*(volatile struct PCMSK1bits_t *)_SFR_MEM_ADDR(PCMSK1))


struct PCMSK2bits_t
{
	uint8_t pcint16:1;
	uint8_t pcint17:1;
	uint8_t pcint18:1;
	uint8_t pcint19:1;
	uint8_t pcint20:1;
	uint8_t pcint21:1;
	uint8_t pcint22:1;
	uint8_t pcint23:1;
};

#define PCMSK2bits (*(volatile struct PCMSK2bits_t *)_SFR_MEM_ADDR(PCMSK2))


struct TIMSK0bits_t
{
	uint8_t	 toie0:1;
	uint8_t ocie0a:1;
	uint8_t ocie0b:1;
	uint8_t		  :5;
};

#define TIMSK0bits (*(volatile struct TIMSK0bits_t *)_SFR_MEM_ADDR(TIMSK0))


struct TIMSK1bits_t
{
	uint8_t	 toie1:1;
	uint8_t ocie1a:1;
	uint8_t ocie1b:1;
	uint8_t		  :2;
	uint8_t	 icie1:1;
	uint8_t		  :2;
};

#define TIMSK1bits (*(volatile struct TIMSK1bits_t *)_SFR_MEM_ADDR(TIMSK1))


struct TIMSK2bits_t
{
	uint8_t	 toie2:1;
	uint8_t ocie2a:1;
	uint8_t ocie2b:1;
	uint8_t		  :5;
};

#define TIMSK2bits (*(volatile struct TIMSK2bits_t *)_SFR_MEM_ADDR(TIMSK2))


// ADCL (ADC Data Register Low) does not have any fields
// ADCH (ADC Data Register High) does not have any fields


union ADCSRAbits_t
{
	struct
	{
		uint8_t adps0:1;
		uint8_t adps1:1;
		uint8_t adps2:1;
		uint8_t	 adie:1;
		uint8_t	 adif:1;
		uint8_t adate:1;
		uint8_t	 adsc:1;
		uint8_t	 aden:1;
	};
	struct
	{
		uint8_t adps:3;
		uint8_t		:5;
	};
};

#define ADCSRAbits (*(volatile union ADCSRAbits_t *)_SFR_MEM_ADDR(ADCSRA))


union ADCSRBbits_t
{
	struct
	{
		uint8_t adts0:1;
		uint8_t adts1:1;
		uint8_t adts2:1;
		uint8_t		 :3;
		uint8_t	 acme:1;
		uint8_t		 :1;
	};
	struct
	{
		uint8_t adts:3;
		uint8_t		:5;
	};
};

#define ADCSRBbits (*(volatile union ADCSRBbits_t *)_SFR_MEM_ADDR(ADCSRB))


union ADMUXbits_t
{
	struct
	{
		uint8_t	 mux0:1;
		uint8_t	 mux1:1;
		uint8_t	 mux2:1;
		uint8_t	 mux3:1;
		uint8_t		 :1;
		uint8_t adlar:1;
		uint8_t refs0:1;
		uint8_t refs1:1;
	};
	struct
	{
		uint8_t	 mux:4;
		uint8_t		:2;
		uint8_t refs:2;
	};
};

#define ACMUXbits (*(volatile union ADMUXbits_t *)_SFR_MEM_ADDR(ADMUX))


struct DIDR0bits_t
{
	uint8_t adc0d:1;
	uint8_t adc1d:1;
	uint8_t adc2d:1;
	uint8_t adc3d:1;
	uint8_t adc4d:1;
	uint8_t adc5d:1;
	uint8_t		 :2;
};

#define DIDR0bits (*(volatile struct DIDR0bits_t *)_SFR_MEM_ADDR(DIDR0))


struct DIDR1bits_t
{
	uint8_t ain0d:1;
	uint8_t ain1d:1;
	uint8_t		 :6;
};

#define DIDR1bits (*(volatile struct DIDR1bits_t *)_SFR_MEM_ADDR(DIDR1))


union TCCR1Abits_t
{
	struct
	{
		uint8_t	 wgm10:1;
		uint8_t	 wgm11:1;
		uint8_t		  :2;
		uint8_t com1b0:1;
		uint8_t com1b1:1;
		uint8_t com1a0:1;
		uint8_t com1a1:1;
	};
	struct
	{
		uint8_t	 wgm1:2;
		uint8_t		 :2;
		uint8_t com1b:2;
		uint8_t com1a:2;
	};
};

#define TCCR1Abits (*(volatile union TCCR1Abits_t *)_SFR_MEM_ADDR(TCCR1A))


union TCCR1Bbits_t
{
	struct
	{
		uint8_t	 cs10:1;
		uint8_t	 cs11:1;
		uint8_t	 cs12:1;
		uint8_t wgm12:1;
		uint8_t wgm13:1;
		uint8_t		 :1;
		uint8_t ices1:1;
		uint8_t icnc1:1;
	};
	struct
	{
		uint8_t cs1:3;
		uint8_t wgm:2;
		uint8_t	   :3;
	};
};

#define TCCR1Bbits (*(volatile union TCCR1Bbits_t *)_SFR_MEM_ADDR(TCCR1B))


struct TCCR1Cbits_t
{
	uint8_t		 :6;
	uint8_t foc1b:1;
	uint8_t foc1a:1;
};


// TCNT1L (Timer 1 Counter Low) does not have any fields
// TCNT1H (Timer 1 Counter High) does not have any fields
// ICR1L (Timer 1 Input Capture Low) does not have any fields
// ICR1H (Timer 1 Input Capture High) does not have any fields
// OCR1AL (Timer 1 Output Compare A Low) does not have any fields
// OCR1AH (Timer 1 Output Compare A High) does not have any fields
// OCR1BL (Timer 1 Output Compare B Low) does not have any fields
// OCR1BH (Timer 1 Output Compare B High) does not have any fields


union TCCR2Abits_t
{
	struct
	{
		uint8_t	 wgm20:1;
		uint8_t	 wgm21:1;
		uint8_t		  :2;
		uint8_t com2b0:1;
		uint8_t com2b1:1;
		uint8_t com2a0:1;
		uint8_t com2a1:1;
	};
	struct
	{
		uint8_t	 wgm0:2;
		uint8_t		 :2;
		uint8_t com2b:2;
		uint8_t com2a:2;
	};
};

#define TCCR2Abits (*(volatile union TCCR2Abits_t *)_SFR_MEM_ADDR(TCCR2A))


union TCCR2Bbits_t
{
	struct
	{
		uint8_t	 cs20:1;
		uint8_t	 cs21:1;
		uint8_t	 cs22:1;
		uint8_t wmg22:1;
		uint8_t		 :2;
		uint8_t foc2b:1;
		uint8_t foc2a:1;
	};
	struct
	{
		uint8_t cs2:3;
		uint8_t	   :5;
	};
};

#define TCCR2Bbits (*(volatile union TCCR2Bbits_t *)_SFR_MEM_ADDR(TCCR2B))


// TCNT2 (Timer 2 Counter) does not have any fields
// OCR2A (Timer 2 Output Compare A) does not have any fields
// OCR2B (Timer 2 Output Compare B) does not have any fields


struct ASSRbits_t
{
	uint8_t tcr2bub:1;
	uint8_t tcr2aub:1;
	uint8_t ocr2bub:1;
	uint8_t ocr2aub:1;
	uint8_t	 tcn2ub:1;
	uint8_t		as2:1;
	uint8_t	  exclk:1;
	uint8_t		   :1;
};

#define ASSRbits (*(volatile struct ASSRbits_t *)_SFR_MEM_ADDR(ASSR))


// TWBR (Two-Wire Bit Rate) does not have any fields


union TWSRbits_t
{
	struct
	{
		uint8_t twps0:1;
		uint8_t twps1:1;
		uint8_t		 :1;
		uint8_t	 tws3:1;
		uint8_t	 tws4:1;
		uint8_t	 tws5:1;
		uint8_t	 tws6:1;
		uint8_t	 tws7:1;
	};
	struct
	{
		uint8_t twps:2;
		uint8_t		:1;
		uint8_t	 tws:5;
	};
};

#define TWSRbits (*(volatile union TWSRbits_t *)_SFR_MEM_ADDR(TWSR))


union TWARbits_t
{
	struct
	{
		uint8_t twgce:1;
		uint8_t	 twa0:1;
		uint8_t	 twa1:1;
		uint8_t	 twa2:1;
		uint8_t	 twa3:1;
		uint8_t	 twa4:1;
		uint8_t	 twa5:1;
		uint8_t	 twa6:1;
	};
	struct
	{
		uint8_t	   :1;
		uint8_t twa:7;
	};
};

#define TWARbits (*(volatile union TWARbits_t *)_SFR_MEM_ADDR(TWAR))


// TWDR (Two-Wire Data Register) does not have any fields


struct TWCRbits_t
{
	uint8_t	 twie:1;
	uint8_t		 :1;
	uint8_t	 twen:1;
	uint8_t	 twwc:1;
	uint8_t twsto:1;
	uint8_t twsta:1;
	uint8_t	 twea:1;
	uint8_t twint:1;
};

#define TWCRbits (*(volatile struct TWCRbits_t *)_SFR_MEM_ADDR(TWCR))


union TWAMRbits_t
{
	struct
	{
		uint8_t		 :1;
		uint8_t twam0:1;
		uint8_t twam1:1;
		uint8_t twam2:1;
		uint8_t twam3:1;
		uint8_t twam4:1;
		uint8_t twam5:1;
		uint8_t twam6:1;
	};
	struct
	{
		uint8_t		:1;
		uint8_t twam:1;
	};
};

#define TWAMRbits (*(volatile union TWAMRbits_t *)_SFR_MEM_ADDR(TWAMR))


struct UCSR0Abits_t
{
	uint8_t mpcm0:1;
	uint8_t	 u2x0:1;
	uint8_t	 upe0:1;
	uint8_t	 dor0:1;
	uint8_t	  fe0:1;
	uint8_t udre0:1;
	uint8_t	 txc0:1;
	uint8_t	 rxc0:1;
};

#define UCSR0Abits (*(volatile struct UCSR0Abits_t *)_SFR_MEM_ADDR(UCSR0A))


struct UCSR0Bbits_t
{
	uint8_t	 txb80:1;
	uint8_t	 rxb80:1;
	uint8_t ucsz02:1;
	uint8_t	 txen0:1;
	uint8_t	 rxen0:1;
	uint8_t udrie0:1;
	uint8_t txcie0:1;
	uint8_t rxcie0:1;
};

#define UCSR0Bbits (*(volatile struct UCSR0Bbits_t *)_SFR_MEM_ADDR(UCSR0B))


union UCSR0Cbits_t
{
	struct
	{
		uint8_t	 ucpol0:1;
		uint8_t	 ucsz00:1;
		uint8_t	 ucsz01:1;
		uint8_t	  usbs0:1;
		uint8_t	  upm00:1;
		uint8_t	  upm01:1;
		uint8_t umsel00:1;
		uint8_t umsel01:1;
	};
	struct
	{
		uint8_t		  :1;
		uint8_t ucpha0:1;
		uint8_t udord0:1;
		uint8_t		  :5;
	};
	struct
	{
		uint8_t		  :1;
		uint8_t	  ucsz:2;
		uint8_t		  :1;
		uint8_t	  upm0:2;
		uint8_t umsel0:2;
	};
};

#define UCSR0Cbits (*(volatile union UCSR0Cbits_t *)_SFR_MEM_ADDR(UCSR0C))


// UBRR0L (USART 0 Baud Rate Register Low) does not have any fields
// UBRR0H (USART 0 Baud Rate Register High) does not have any fields
// UDR0 (USART 0 Date Register) does not have any fields


#endif // INCLUDE_REGMAP_H_
