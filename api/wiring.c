#include <wiring.h>

static volatile uint32_t __millis = 0;

#define N_PINS (sizeof(_pins) / sizeof (_pins[0]))

uint32_t millis() {
    return __chipGetMillisTick();
}

void delay(uint32_t del) {
    uint32_t start = millis();
    while (millis() - start < del) {
        savePower();
    }
}

void __initSystem() {
    __chipInitSystem();
    __chipInitMillisTimer();

#if defined(_HAS_INITPLL)
    extern void __initPLL();
    __initPLL();
#endif
}

// Put the system to sleep until the next "tick" of the timer.  This will
// sleep for up to 1 ms.
void savePower() {
#if defined(_PIC18)
    OSCCONbits.IDLEN = 1;
    OSCCONbits.SCS = 0b00;
    asm("SLEEP");
#endif
}

