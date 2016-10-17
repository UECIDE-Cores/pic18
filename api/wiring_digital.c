#include <wiring.h>

static volatile uint32_t __millis = 0;

#define N_PINS (sizeof(_pins) / sizeof (_pins[0]))

void pinMode(uint8_t pin, uint8_t mode) {
    if (pin >= _pins_max) {
        return;
    }

    switch(mode) {
        case OUTPUT:
            __chipSetPinFunction(__ports[pin], __pins[pin], __CHIP_PIN_OUTPUT);
            break;

        case INPUT_PULLUP:
            __chipSetPinFunction(__ports[pin], __pins[pin], __CHIP_PIN_INPUT_PULLUP);
            break;

        case INPUT_PULLDOWN:
            __chipSetPinFunction(__ports[pin], __pins[pin], __CHIP_PIN_INPUT_PULLDOWN);
            break;

        case INPUT:
            __chipSetPinFunction(__ports[pin], __pins[pin], __CHIP_PIN_INPUT);
            break;
    }
}

void digitalWrite(uint8_t pin, uint8_t val) {
    if (pin >= _pins_max) {
        return;
    }
    __chipSetDigitalValue(__ports[pin], __pins[pin], val);
}

uint8_t digitalRead(uint8_t pin) {
    if (pin >= _pins_max) {
        return LOW;
    }
    return __chipGetDigitalValue(__ports[pin], __pins[pin]) ? HIGH : LOW;
}

