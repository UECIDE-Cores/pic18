#ifndef _CHIP_H
#define _CHIP_H

/* Common chip pin functions */
#define __CHIP_PIN_OUTPUT           0x00
#define __CHIP_PIN_OUTPUT_ANALOG    0x04

#define __CHIP_PIN_INPUT            0x10
#define __CHIP_PIN_INPUT_PULLUP     0x20
#define __CHIP_PIN_INPUT_PULLDOWN   0x30
#define __CHIP_PIN_INPUT_ANALOG     0x40

/* Chip abstraction prototypes */

extern void __chipSetPinFunction(uint8_t, uint8_t, uint8_t);
extern void __chipSetDigitalValue(uint8_t, uint8_t, uint8_t);
extern uint8_t __chipGetDigitalValue(uint8_t, uint8_t);
extern void __chipSetAnalogValue(uint8_t, uint8_t, int16_t);
extern int16_t __chipGetAnalogValue(uint8_t, uint8_t);
extern void __chipInitMillisTimer();
extern void __chipInitSystem();
extern uint32_t __chipGetMillisTick();

#endif
