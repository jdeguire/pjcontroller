/* deviceprog.h
 *
 * Routines for dealing with the onboard flash and EEPROM memory.
 */

#ifndef INCLUDE_DEVICEPROG_H_
#define INCLUDE_DEVICEPROG_H_

#include "sharedmem.h"
#include <stdint.h>
#include <stdbool.h>

void Flash_WriteLatch(uint16_t offset, uint16_t insword);
void Flash_ProgramPage(uint16_t addr);
bool Flash_VerifyPage(uint16_t page, uint16_t *buf);
void Flash_EraseApp();
void EEPROM_EraseData();

#endif // INCLUDE_DEVICEPROG_H_
