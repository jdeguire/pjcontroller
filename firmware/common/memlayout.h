/* memlayout.h
 *
 * Contains constructs and functions pertaining to the device's memory layout that would be used by
 * both the bootloader and the application, such as the end of app space and a function for
 * calculating the checksum of the application.
 */

#ifndef INCLUDE_MEMLAYOUT_H_
#define INCLUDE_MEMLAYOUT_H_

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

// These are defined in the custom linker scripts for the app and bootloader.
// Their addresses are the important part.
extern uint8_t PROGMEM _app_space_start;
extern uint8_t PROGMEM _app_space_end;
extern uint8_t PROGMEM _ivt_start;
extern uint8_t PROGMEM _ivt_end;


#define APP_SPACE_START ((uint16_t)&_app_space_start)
#define APP_SPACE_END   ((uint16_t)&_app_space_end)

#define NUM_APP_PAGES   (((uint16_t)&_app_space_end + (SPM_PAGESIZE - 1)) / SPM_PAGESIZE)

// The bootloader will write the app's info (see below) to the address immediately following the
// app's IVT.  This macro will allow the app and bootloader to figure out where exactly that is,
// assuming that the app and bootloader have the same size IVT.
#define APPINFO_ADDR  ((uint16_t)&_ivt_end - (uint16_t)&_ivt_start)


typedef struct appinfo_tag
{
	uint16_t valid;           // set to 0xAA55 to indicate this data is valid
	uint16_t last_page;       // last page containing app data
	uint16_t checksum;        // 16-bit crc validated by bootloader on startup
} appinfo_t;

#define APPINFO_VALID 0xAA55

#endif // INCLUDE_MEMLAYOUT_H_
