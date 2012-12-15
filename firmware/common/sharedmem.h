/* sharedmem.h
 *
 * Used for routines and memory shared between the PJC Bootloader and the application.
 */

#ifndef INCLUDE_SHAREDMEM_H_
#define INCLUDE_SHAREDMEM_H_

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

// These are defined in the custom linker scripts for the app and bootloader.
// Their addresses are the important part.
extern const __flash uint8_t _app_space_start;
extern const __flash uint8_t _app_space_end;

#define APP_SPACE_START ((uint16_t)&_app_space_start)
#define APP_SPACE_END   ((uint16_t)&_app_space_end)

#define NUM_APP_PAGES   ((APP_SPACE_END + (SPM_PAGESIZE - 1)) / SPM_PAGESIZE)

bool AppRestartRequested();
void ClearAppRestartRequest();
void RestartApp() __attribute__((noreturn));
void RestartBootloader() __attribute__((noreturn));
uint16_t CalculateAppCRC();

#endif // INCLUDE_SHAREDMEM_H_
