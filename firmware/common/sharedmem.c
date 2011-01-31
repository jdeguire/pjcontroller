/* sharedmem.c
 *
 * The bootloader and application need to be able to share parts of program memory and RAM.  For
 * example, the bootloader and app need to be able to access a portion of flash where the app's
 * checksum will be stored.  Also, a location in RAM is reserved so that the app can tell the
 * bootloader to reload the app on startup.
 *
 * Functions for dealing with app space and these shared spaces are here, such as a function to jump
 * to the bootloader, restart the app, and get the application's checksum.
 */

#include "sharedmem.h"
#include "watchdog.h"
#include "misc.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/crc16.h>


// Not initialized by the startup module, so that memory contents are retained after a watchdog
// reset.  Note that the memory state is not trustworthy for other reset sources.
static volatile bool m_loadApplication __attribute__((section(".shareddata")));


/* Return True if the application reset the device (using the watchdog) and requested that the
 * bootloader restart the app.  If the device was _not_ reset by the watchdog, then restart the app
 * so that powering up the device runs the app as exptected.
 */
bool AppRestartRequested()
{
	return (!ResetByWatchdog()  ||  (ResetByWatchdog()  &&  m_loadApplication));
}

/* Clear the request to restart the app on reset.  This is done in the bootloader so that an
 * unintentional watchdog reset will land in the bootloader rather than restarting the (probably)
 * faulty application.
 */
void ClearAppRestartRequest()
{
	m_loadApplication = false;
}

/* Restart the application by requesting the bootloader to restart the app, enabling the shortest
 * watchdog time, and looping until the device restarts.  Note that other conditions (such as a bad
 * checksum) can still cause the bootloader to not start the app.
 */
void RestartApp()
{
	m_loadApplication = true;
    wdt_enable(WDTO_15MS);
	for(;;) ;
}

/* Restart and stay in the bootloader by enabling the shortest watchdog timer and running an
 * infinite loop.
 */
void RestartBootloader()
{
	m_loadApplication = false;
    wdt_enable(WDTO_15MS);
	for(;;)	;
}

/* Calculate 16-bit CRC for everything in app space.
 */
uint16_t CalculateAppCRC()
{
	uint16_t crc = 0xFFFF;
	uint16_t addr;

	for(addr = APP_SPACE_START; addr <= APP_SPACE_END; ++addr)
		crc = _crc_ccitt_update(crc, pgm_read_byte(addr));

	return crc;
}
