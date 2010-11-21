/* uart.c
 * 
 * Contains functions for setting up the UART module and transacting data.  There are separate send
 * and receive circular buffers to store data and interrupts for receiving data into and sending
 * data out of the buffers.
 */

#include "regmap.h"
#include "uart.h"
#include <avr/interrupt.h>
#include <string.h>

// these assume normal speed asynchronous mode
#define BAUD2REG (((F_CPU / (8 * BAUDRATE)) - 1) / 2)       // rounds to nearest baud reg value
#define REG2BAUD (F_CPU / (16 * (BAUD2REG + 1)))            // baud reg value to actual rate

#define BAUDERROR ((1000L*REG2BAUD / BAUDRATE) - 1000L)     // gives per mil rather than percent

#if (BAUDERROR > 20  ||  BAUDERROR < -20)
#  error Baud rate error greater than 2%.  See Section 19 of the Reference Manual.
#endif


static volatile char m_txbuffer[UART_TX_BUFSIZE];
static volatile uint8_t m_txstart = 0;
static volatile uint8_t m_txcount = 0;

static volatile char m_rxbuffer[UART_RX_BUFSIZE];
static volatile uint8_t m_rxstart = 0;
static volatile uint8_t m_rxcount = 0;


/* Setup the UART device.  The baud rate is set to the value above, the module is enabled and set to
 * the proper settings, and the UART recieve interrupt is enabled.  The transmit interrupt is
 * enabled when we want to transmit data.
 */
void UART_Init()
{
	UBRR0H = (BAUD2REG >> 8) & 0x0F;    // only lower 4 bits of UBRR0H are valid
	UBRR0L = BAUD2REG & 0xFF;

	UCSR0A = 0;
	UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);                 // 8-N-1

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

/* Put a string of length 'len' into the buffer or however many characters will fit.  Returns the
 * number of characters sent.
 */
uint8_t UART_TxData(char *data, uint8_t len)
{
	uint8_t i;

	for(i = 0; i < len; i++)
	{
		if(false == UART_TxChar(data[i]))
			break;
	}

	return i;
}

/* Returns 0 if no characters are available. 
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

/* Returns the number of characters received.
 */
uint8_t UART_RxData(char *data, uint8_t len)
{
	uint8_t i;
	
	for(i = 0; i < len  &&  m_rxcount > 0; i++)
		data[i] = UART_RxChar();

	return i;
}

/* Number of spaces left in the TX buffer for new data.
 */
uint8_t UART_TxAvailable()
{
	return UART_TX_BUFSIZE - m_txcount;
}

/* Number of characters in the receive buffer.
 */
uint8_t UART_RxAvailable()
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
