/* deviceprog.h
 *
 * Routines for dealing with the onboard flash and EEPROM memory.
 */

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>

#define NUM_APP_PAGES  224
#define Flash_PageSize() (SPM_PAGESIZE)
#define Flash_NumPages() (NUM_APP_PAGES)

// avr-libc functions use byte addresses for its functions
#define APP_SPACE_START  0              // application data
#define APP_SPACE_END    0x6F7F
#define APP_ID_START     0x6F80         // application checksum and version string
#define APP_ID_END       0x6FFF
#define BOOT_SPACE_START 0x7000         // bootloader data
#define BOOT_SPACE_END   0x7F7F
#define BOOT_ID_START    0x7F80         // bootloader version string
#define BOOT_ID_END      FLASHEND


void Flash_ProgramPage(uint16_t page, uint16_t *buf);
bool Flash_VerifyPage(uint16_t page, uint16_t *buf);
uint16_t Flash_CalculateAppCRC();
void Flash_EraseApp();
void EEPROM_EraseData();
