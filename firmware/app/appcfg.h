/* appcfg.h
 *
 * Project-specific defines and settings used in multiple places.  This file is included in several
 * common code modules as well, allowing you to change their settings.  For example, you could
 * define the UART_RX_BUFSIZE macro to change the size of the UART receive buffer.
 */

#ifndef INCLUDE_APPCFG_H_
#define INCLUDE_APPCFG_H_

#include "misc.h"

#define CMD_MAXCMDS 20

#define FIRMWARE_VERSION 0
#define VERSION_STRING "Projector Controller v"STRINGIFY(FIRMWARE_VERSION)


#endif // INCLUDE_APPCFG_H_
