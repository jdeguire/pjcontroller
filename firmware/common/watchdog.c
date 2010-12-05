/* watchdog.c
 *
 * Utilities for the AVR's watchdog timer.
 */

#include "watchdog.h"
#include <stdint.h>
#include <avr/io.h>

static void get_mcusr(void) __attribute__((used, naked, section(".init3")));

// in .noinit because this is set before the initialization stuff happens
static uint8_t m_mcusr_mirror __attribute__ ((section (".noinit")));


/* The watchdog is still enabled after a watchdog reset, so this will disable it for us early on in
 * the startup process so that the device is not stuck in a reset loop.  This function and the above
 * variable declaration were adapted from the avr-libc manual here:
 * http://www.nongnu.org/avr-libc/user-manual/group__avr__watchdog.html
 */
static void get_mcusr(void)
{
	m_mcusr_mirror = MCUSR;
	MCUSR = 0;
	wdt_disable();
}

/* Return True if the device was reset by the watchdog timer.  This means that only the watchdog
 * reset flag, and no others, is set in the MCUSR register.
 */
bool ResetByWatchdog()
{
	return (m_mcusr_mirror == (1 << WDRF));
}
