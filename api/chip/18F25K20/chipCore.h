#ifndef _CHIP_CORE_H
#define _CHIP_CORE_H

#include <stdint.h>

/* PIC18F25K20
 * 
 * 28 pin DIP/SOIC/SSOP/QFN.
 */

// Definitions for the chip peripherals

#define PORT_A 1
#define PORT_B 2
#define PORT_C 3
// There is no port D.
// Port E only has RE3 on MCLR if MCLR is disabled.
#define PORT_E 5

#endif
