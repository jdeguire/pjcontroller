/* localcmd.c
 *
 * Serial interface commmands specific to the bootloader.  See common/cmd.c for the implementation
 * of the interface itself.
 */

#include "localcmd.h"
#include "cmd.h"
#include "sharedmem.h"
#include "uart.h"
#include "misc.h"
#include "watchdog.h"
#include <stdint.h>
#include <stdbool.h>

static void JumpToApp_CMD(const char *cmdbuf, uint8_t len) __attribute__((noreturn));
static void RestartBootloader_CMD(const char *cmdbuf, uint8_t len) __attribute__((noreturn));


/* Jump to the bootloader at the end of flash.  This assumes a bootloader is present.
 * Syntax: j
 * Response: none (will see startup message after jump)
 */
static void JumpToBootloader_CMD(const char *cmdbuf, uint8_t len)
{
	RestartBootloader();
}

/* Reset the device and try to start the app when it restarts.
 * Syntax: r
 * Response: none (will see startup message after reset)
 */
static void RestartApp_CMD(const char *cmdbuf, uint8_t len)
{
	RestartApp();
}

/* Call this once at startup to register the above command handlers and associate that with commands
 * to be called from a serial console.
 */
void RegisterAppCommands()
{
	Cmd_RegisterCommand("j", JumpToBootloader_CMD, PSTR("Jump to bootloader"));
	Cmd_RegisterCommand("r", RestartApp_CMD, PSTR("Restart app"));
}
