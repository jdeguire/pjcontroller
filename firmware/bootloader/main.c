/* main.c
 *
 * Main code module for the bootloader.
 */

#include "regmap.h"
#include "uart.h"
#include "cmd.h"
#include "sharedmem.h"
#include "deviceprog.h"
#include "watchdog.h"
#include <stdint.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* Stuff to do in main():
 *
 * Startup:
 * app inhibit pin set?
 * --yes, print message and stay in bootloader
 * --no, continue
 * app restart requested?
 * --yes, continue
 * --no, print message and stay in bootloader
 * check for valid app crc
 * --if there, continue
 * --if not, print message, set LED, and start bootloader
 * compute and validate app crc
 * --match, start app
 * --mismatch, print message, set LED, and start bootloader
 *
 * Start Bootloader:
 * Enable watchdog
 * Enable UART
 * Enable command interface
 * 
 * Jump to app (only if bootloader was started):
 * Reset UART, LEDs,
 * Disable interrupts, watchdog
 * Reset IVT to app space
 */

typedef enum
{
	eBootOK      = 0,
	eBootPinSet,
	eBootRestart,
	eBootNoApp,
	eBootCRC
} bootstatus_t;

/* Get the state of the Bootloader Pin.  If True, stay in the bootloader without attempting to start
 * the app.  Otherwise, try to start the application.
 */
static bool BootloaderPinSet()
{
	// we'll use an external pull-down for this
	DDRBbits.ddb0 = 0;        // 0 for input
	return PINBbits.pinb0;
}

/* Check if there is any reason why the bootloader should not start the app.  Returns 'eBootOK' if
 * the app can run or a non-zero status if it cannot.
 */
static bootstatus_t GetAppStartupStatus()
{
	bootstatus_t status = eBootOK;

	if(BootloaderPinSet())
		status = eBootPinSet;
	else if(!AppRestartRequested())
		status = eBootRestart;
	else
	{
		uint16_t crcinfo[2];

		memcpy_P((void *)crcinfo, (PGM_VOID_P)(FLASHEND - 4), 4);

		if(crcinfo[0] != APP_CHECKSUM_VALID)
			status = eBootNoApp;
		else if(crcinfo[1] != CalculateAppCRC())
			status = eBootCRC;
	}

	ClearAppRestartRequest();

	return status;
}

/* Turn on or off LED used as a visual notice that the board is in the bootloader.
 */
static void LightBootloaderLED(bool enable)
{
	if(enable)
	{
		DDRDbits.ddd6 = 1;   // output
		PORTDbits.pd6 = 1;   // turn on
	}
	else
	{
		// return to defaults
		PORTDbits.pd6 = 0;
		DDRDbits.ddd6 = 0;
	}
}

/* True enables the bootloader IVT and False returns to the app's IVT.  Set the False before jumping
 * to the application.
 */
static void SetBootIVT(bool bootIVT)
{
	if(bootIVT)
	{
		MCUCR = (1 << IVCE);
		MCUCR = (1 << IVSEL);
	}
	else
	{
		MCUCR = (1 << IVCE);
		MCUCR = 0;
	}
}

/* Undo the modifications to registers that the bootloader did when starting up.  Do this before
 * jumping to the app after the bootloader has started up.
 */
void RewindSettings()
{
	cli();                    // disable interrupts
	wdt_reset();
	SetBootIVT(false);
	LightBootloaderLED(false);

	// initial UART register values
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	UCSR0B = 0;
	UCSR0A = 0;
	UBRR0H = 0;
	UBRR0L = 0;

	wdt_disable();
}

int main(void)
{
	bootstatus_t status = GetAppStartupStatus();

	if(status == eBootOK)
		asm volatile("jmp 0x0");
	else
	{
		wdt_enable(WDTO_250MS);

		UART_Init();
		Cmd_InitInterface();

		SetBootIVT(true);
		sei();                    // enable interrupts

		LightBootloaderLED(true);
		wdt_reset();

		while(true)
		{
			wdt_reset();
			Cmd_ProcessInterface();
		}
	}
}
