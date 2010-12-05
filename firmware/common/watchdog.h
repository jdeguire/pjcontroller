/* watchdog.h
 *
 * Utilities for the AVR's watchdog timer.
 */

#include <stdbool.h>
#include <avr/wdt.h>

bool ResetByWatchdog();
