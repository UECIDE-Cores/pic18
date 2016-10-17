#include <wiring.h>

uint16_t analogRead(uint8_t pin) {
    if (pin >= _pins_max) {
        return;
    }

    __chipSetPinFunction(__ports[pin], __pins[pin], __CHIP_PIN_INPUT_ANALOG);
    return __chipGetAnalogValue(__ports[pin], __pins[pin]);
}

void analogWrite(uint8_t pin, uint8_t duty) {
    if (pin >= _pins_max) {
        return;
    }
    __chipSetAnalogValue(__ports[pin], __pins[pin], duty);
}
