/* system.h
 *
 * Project-specific defines and settings used in multiple places.  This file is included in several
 * common code modules as well, allowing you to change their settings.  For example, you could
 * define the UART_RX_BUFSIZE macro to change the size of the UART receive buffer.
 */

#ifndef INCLUDE_SYSTEM_H_
#define INCLUDE_SYSTEM_H_

#include "misc.h"

#define __PJC_BOOTLOADER__

#define APP_CHECKSUM_VALID 0xAA55

#define MAX_CMDS 12

#define FIRMWARE_VERSION 0
#define VERSION_STRING "PJC Bootloader v"STRINGIFY(FIRMWARE_VERSION)


#endif // INCLUDE_SYSTEM_H_
