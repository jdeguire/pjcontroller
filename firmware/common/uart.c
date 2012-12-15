/* uart.c
 * 
 * Contains functions for setting up the UART module and transacting data.  There are separate send
 * and receive circular buffers to store data and interrupts for receiving data into and sending
 * data out of the buffers.
 */

#include "uart.h"
#include <avr/interrupt.h>
#include <string.h>

// these assume normal speed asynchronous mode
#define BAUD2REG (((F_CPU / (8 * UART_BAUDRATE)) - 1) / 2)       // rounds to nearest baud reg value
#define REG2BAUD (F_CPU / (16 * (BAUD2REG + 1)))                 // baud reg value to actual rate

#define BAUDERROR ((1000L*REG2BAUD / UART_BAUDRATE) - 1000L)     // gives per mil rather than percent

#if (BAUDERROR > 20  ||  BAUDERROR < -20)
#  error Baud rate error greater than 2%.  See Section 19 of the Reference Manual.
#endif


static volatile char m_txbuffer[UART_TX_BUFSIZE];
static volatile uint16_t m_txstart = 0;
static volatile uint16_t m_txcount = 0;

static volatile char m_rxbuffer[UART_RX_BUFSIZE];
static volatile uint16_t m_rxstart = 0;
static volatile uint16_t m_rxcount = 0;


/* Setup the UART device.  The baud rate is set to what the BUADRATE macro is defined as, the module
 * is enabled and set to the proper settings, and the UART recieve interrupt is enabled.  The
 * transmit interrupt is enabled when we want to transmit data.
 */
void UART_Init()
{
	UBRR0H = (BAUD2REG >> 8) & 0x0F;    // only lower 4 bits of UBRR0H are valid
	UBRR0L = BAUD2REG & 0xFF;

	UCSR0A = 0;
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);                 // 8-N-1
	UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);

	m_txstart = 0;
	m_txcount = 0;
	m_rxstart = 0;
	m_rxcount = 0;
}


/* Put a character into the buffer if there is room.  Returns True if it was able to do so.
 */
bool UART_TxChar(char c)
{
	bool result = false;

    _UART_TXIE(0);
	if(m_txcount < UART_TX_BUFSIZE)
	{
		m_txbuffer[(m_txstart + m_txcount) % UART_TX_BUFSIZE] = c;
		++m_txcount;
		result = true;
	}
  	_UART_TXIE(1);

	return result;
}

/* Transmit a single byte to be displayed as a hexadecimal number.  So 0x36 would be sent as "36"
 * rather than as the ASCII character '6'.  Be aware of byte order when sending bytes from an
 * integral value.  Returns True if the the hex digits could be sent and False otherwise.
 */
bool UART_TxHexByte(uint8_t by)
{
	bool result = false;

	if(UART_TxAvailable() >= 2)
	{
		uint8_t lower = by & 0x0F;
		uint8_t upper = (by & 0xF0) >> 4;

		if(upper > 9)
			UART_TxChar(upper + 'A' - 10);
		else
			UART_TxChar(upper + '0');

		if(lower > 9)
			UART_TxChar(lower + 'A' - 10);
		else
			UART_TxChar(lower + '0');

		result = true;
	}
	return result;
}

/* Put a string of length 'len' into the buffer or however many characters will fit.  Returns the
 * number of characters sent.
 */
uint16_t UART_TxData(const char *data, uint16_t len)
{
	uint16_t i;

	for(i = 0; i < len; i++)
	{
		if(false == UART_TxChar(data[i]))
			break;
	}

	return i;
}

/* Same as above, but the data string is in program space.
 */
uint16_t UART_TxData_P(const __flash char *data, uint16_t len)
{
	uint16_t i;

	for(i = 0; i < len; i++)
	{
		if(false == UART_TxChar(pgm_read_byte(data+i)))
			break;
	}

	return i;
}


/* Remove and return the next character from the receive buffer.  Returns 0 if no characters are
 * available.
 */
char UART_RxChar()
{
	_UART_RXIE(0);
	char c = 0;
	if(m_rxcount > 0)
	{
		c = m_rxbuffer[m_rxstart];
		m_rxstart = (m_rxstart + 1) % UART_RX_BUFSIZE;
		--m_rxcount;
	}
	_UART_RXIE(1);
	
	return c;
}

/* Return the number of characters received and remove those characters from the buffer.
 */
uint16_t UART_RxData(char *data, uint16_t len)
{
	uint16_t i;
	
	for(i = 0; i < len  &&  m_rxcount > 0; i++)
		data[i] = UART_RxChar();

	return i;
}


/* Number of spaces left in the TX buffer for new data.
 */
uint16_t UART_TxAvailable()
{
	return UART_TX_BUFSIZE - m_txcount;
}

/* Number of characters in the receive buffer.
 */
uint16_t UART_RxAvailable()
{
	return m_rxcount;
}


/* Interrupt handler for UART Data Register Empty flag, which is used during transmission of data.
 * Do not call this directly.
 */
ISR(USART_UDRE_vect)
{
	UDR0 = m_txbuffer[m_txstart];
	m_txstart = (m_txstart + 1) % UART_TX_BUFSIZE;
	--m_txcount;

	// disable transmit interrupt if there isn't anything left to transmit
	if(0 == m_txcount)
		_UART_TXIE(0);
}

/* Interrupt handler for UART receives.  Do not call this directly.
 */
ISR(USART_RX_vect)
{
	m_rxbuffer[(m_rxstart + m_rxcount) % UART_RX_BUFSIZE] = UDR0;
	++m_rxcount;
}
