/* misc.h
 *
 * Stuff that doesn't really belong anywhere else.
 */

#ifndef INCLUDE_MISC_H_
#define INCLUDE_MISC_H_

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define STRINGIFY_(s) #s
#define STRINGIFY(s) STRINGIFY_(s)

#endif // INCLUDE_MISC_H_