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
#include <stdlib.h>

// Not initialized by the startup module, so that memory contents are retained after a watchdog
// reset.  Note that the memory state is not trustworthy for other reset sources.
static volatile bool m_loadApplication __attribute__((section(".shareddata")));


/* Return True if the application reset the device (using the watchdog) and requested that the
 * bootloader restart the app.  This is done using the RestartApp() function below.
 */
bool AppRestartRequested()
{
	return (ResetByWatchdog()  &&  m_loadApplication);
}

/* Clear the request to restart the app on reset.  This is done so that an unintentional watchdog
 * reset will land in the bootloader rather than restarting the (probably) faulty application.
 */
void ClearAppRestartRequest()
{
	m_loadApplication = 0;
}

/* Restart the application by requesting the bootloader to restart the app, enabling the shortest
 * watchdog time, and calling exit(), which goes to an infinite loop.  Note that other conditions
 * (such as a bad checksum) can still cause the bootloader to not start the app.
 */
void RestartApp()
{
	m_loadApplication = true;
    wdt_enable(WDTO_15MS);
	exit(0);
}

/* Restart and stay in the bootloader by enabling the shortest watchdog timer and calling exit(),
 * which just goes to an infinite loop.
 */
void RestartBootloader()
{
	m_loadApplication = false;
    wdt_enable(WDTO_15MS);
	exit(0);
}

/* Read the app info data from program memory.  The data is written to flash after the bootloader
 * has finished writing the new application.
 */
void GetAppInfo(appinfo_t *appinfo)
{
	memcpy_P(appinfo, (appinfo_t *)APPINFO_ADDR, sizeof(appinfo_t));
}

/* Calculate 16-bit CRC for the application.  This does not include unused pages after the
 * application's end nor the area in which the appinfo_t structure is stored.
 */
uint16_t CalculateAppCRC()
{
	appinfo_t appinfo;
	uint16_t crc = 0xFFFF;
	uint16_t addr;
	uint16_t end_addr = APP_SPACE_END + 1;

	GetAppInfo(&appinfo);

	if(APPINFO_VALID == appinfo.valid)
		end_addr = MIN(appinfo.num_pages * SPM_PAGESIZE, APP_SPACE_END + 1);

	for(addr = APP_SPACE_START; addr < end_addr; ++addr)
	{
		if(addr < APPINFO_ADDR  &&  addr > (APPINFO_ADDR + sizeof(appinfo_t)))
			crc = _crc_ccitt_update(crc, pgm_read_byte(addr));
	}
	return crc;
}
