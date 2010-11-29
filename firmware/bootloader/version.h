/* version.h
 *
 * Version number and ID string for the bootloader.
 */

#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

#define PROG_VERSION 0
#define VERSION_STRING "PJC Bootloader v"STRINGIFY(PROG_VERSION)
