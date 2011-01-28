/* sharedmem.h
 *
 * The bootloader and application need to be able to share parts of program memory and RAM.  For
 * example, the bootloader and app need to be able to access a portion of flash where the app's
 * checksum will be stored.  Also, a location in RAM is reserved so that the app can tell the
 * bootloader to reload the app on startup.
 *
 * Functions for dealing with app space and these shared spaces are here, such as a function to jump
 * to the bootloader, restart the app, and get the application's checksum.
 */

#ifndef INCLUDE_SHAREDMEM_H_
#define INCLUDE_SHAREDMEM_H_

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

// These are defined in the custom linker scripts for the app and bootloader.
// Their addresses are the important part.
extern prog_uint8_t _app_space_start;
extern prog_uint8_t _app_space_end;
extern prog_uint8_t _ivt_start;
extern prog_uint8_t _ivt_end;


#define APP_SPACE_START ((uint16_t)&_app_space_start)
#define APP_SPACE_END   ((uint16_t)&_app_space_end)

#define MAX_APP_PAGES   ((APP_SPACE_END + (SPM_PAGESIZE - 1)) / SPM_PAGESIZE)

// The bootloader will write the app's info (see below) to the address immediately following the
// app's IVT.  This macro will allow the app and bootloader to figure out where exactly that is,
// assuming that the app and bootloader have the same size IVT.
#define APPINFO_ADDR  ((uint16_t)&_ivt_end - (uint16_t)&_ivt_start)


typedef struct appinfo_tag
{
	uint16_t valid;           // set to 0xAA55 to indicate this data is valid
	uint16_t num_pages;       // number of pages the app spans
	uint16_t crc16;           // 16-bit crc validated by bootloader on startup
} appinfo_t;

#define APPINFO_VALID 0xAA55

bool AppRestartRequested();
void ClearAppRestartRequest();
void RestartApp() __attribute__((noreturn));
void RestartBootloader() __attribute__((noreturn));
void GetAppInfo(appinfo_t *appinfo);
uint16_t CalculateAppCRC();

#endif // INCLUDE_SHAREDMEM_H_
