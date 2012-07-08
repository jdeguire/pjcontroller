/* localcmd.h
 *
 * Serial interface commmands specific to the bootloader.  See common/cmd.c for the implementation
 * of the interface itself.
 */

#ifndef INCLUDE_LOCALCMD_H_
#define INCLUDE_LOCALCMD_H_

#define APP_CHECKSUM_VALID 0xAA55

void RegisterBootloaderCommands();

#endif // INCLUDE_LOCALCMD_H_
