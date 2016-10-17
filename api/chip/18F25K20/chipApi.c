#include <chipCore.h>
#include <stdio.h>
#include <xc.h>
#include <api.h>

static volatile uint8_t *__tris[] = { NULL, &TRISA, &TRISB, &TRISC };
static volatile uint8_t *__port[] = { NULL, &PORTA, &PORTB, &PORTC };
static volatile uint8_t *__lat[] = { NULL, &LATA, &LATB, &LATC };

static uint32_t __millis = 0;

static uint8_t __pwmActive = 0;

static void interrupt high_priority __isr_hi_handler(void) {
    if (INTCONbits.TMR0IE && INTCONbits.T0IF) {
        INTCONbits.T0IF = 0;
        uint16_t tpr = (F_CPU/4)/1000;
        tpr = 0xFFFF - tpr;
        TMR0H = tpr >> 8;
        TMR0L = tpr & 0xFF;
        __millis++;
    }
}

static void interrupt low_priority __isr_lo_handler(void) {
#ifdef _HAS_EUSART
    if (PIE1bits.RCIE && PIR1bits.RCIF) {
        PIR1bits.RCIF = 0;
        Serial.ungetc(RCREG);
    }
#endif
}

static uint8_t portPinToAnalogChannel(uint8_t port, uint8_t pin) {
    switch(port) {
        case PORT_A:
            switch(pin) {
                case 0: return 0;
                case 1: return 1;
                case 2: return 2;
                case 3: return 3;
                case 5: return 4;
            }
            break;
        case PORT_B:
            switch(pin) {
                case 0: return 12;
                case 1: return 10;
                case 2: return 8;
                case 3: return 9;
                case 4: return 11;
            }
            break;
    }
    return 255;
}

static const uint8_t t2prescale[] = {1, 4, 16};

static void enableTimerTwo() {
    T2CONbits.T2CKPS = 3;
    T2CONbits.T2OUTPS = 2;
    PR2 = 0xFF;
    T2CONbits.TMR2ON = 1;
}

void __chipInitMillisTimer() {
    // Set up TIMER0 to tick at 1ms intervals.
    // The oscillator ticks at Fosc/4, so 4MHz. That is 1/16000000 s per tick.
    // or 0.000000063s, or 0.000063ms, so 1 ms is 16000 ticks.
    T0CONbits.T08BIT = 0; // 16-bit
    T0CONbits.T0CS = 0; // Internal clock
    T0CONbits.PSA = 1; // No prescaler
    uint16_t tpr = (F_CPU/4)/1000;
    tpr = 0xFFFF - tpr;
    TMR0H = tpr >> 8;
    TMR0L = tpr & 0xFF;
    INTCONbits.T0IF = 0;
    INTCONbits.T0IE = 1;
    INTCON2bits.TMR0IP = 1;
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
    T0CONbits.TMR0ON = 1;
}

uint32_t __chipGetMillisTick() {
    uint32_t mil;
    INTCONbits.T0IE = 0;
    mil = __millis;
    INTCONbits.T0IE = 1;
    return mil;
}

void __chipInitSystem() {
    INTCON2bits.RBPU = 0;
}

void __chipSetPinFunction(uint8_t port, uint8_t pin, uint8_t function) {
    uint8_t an = 255;
    switch (function) {
        case __CHIP_PIN_OUTPUT:
            if (port == PORT_C) {
                if ((pin == 2) & (__pwmActive & 0x01)) {
                    CCP1CONbits.CCP1M = 0;
                    __pwmActive &= 0xFE;
                    if (__pwmActive == 0) {
                        T2CONbits.TMR2ON = 0;
                    }
                } else if ((pin == 1) && (__pwmActive & 0x02)) {
                    CCP2CONbits.CCP2M = 0;
                    __pwmActive &= 0xFD;
                    if (__pwmActive == 0) {
                        T2CONbits.TMR2ON = 0;
                    }
                }
            }
            *__tris[port] &= ~(1 << pin);
            break;

        case __CHIP_PIN_OUTPUT_ANALOG:
            if ((port == PORT_C) && ((pin == 1) || (pin == 2))) {
                *__tris[port] &= ~(1 << pin);
                if (__pwmActive == 0) {
                    enableTimerTwo();
                }
                if (pin == 2) {
                    CCP1CONbits.CCP1M = 0x0F;
                    __pwmActive |= 1;
                } else {
                    CCP2CONbits.CCP2M = 0x0F;
                    __pwmActive |= 2;
                }
            }
            break;

        case __CHIP_PIN_INPUT:
        case __CHIP_PIN_INPUT_PULLDOWN: // Pulldowns not supported.
            *__tris[port] |= (1 << pin);
            if (port == PORT_C) {
                if ((pin == 2) & (__pwmActive & 0x01)) {
                    CCP1CONbits.CCP1M = 0;
                    __pwmActive &= 0xFE;
                    if (__pwmActive == 0) {
                        T2CONbits.TMR2ON = 0;
                    }
                } else if ((pin == 1) && (__pwmActive & 0x02)) {
                    CCP2CONbits.CCP2M = 0;
                    __pwmActive &= 0xFD;
                    if (__pwmActive == 0) {
                        T2CONbits.TMR2ON = 0;
                    }
                }
            }

            an = portPinToAnalogChannel(port, pin);
            if (an != 255) {
                if (an < 8) {
                    ANSEL &= ~(1 << an);
                } else {
                    ANSELH &= ~(1 << (an - 8));
                }
            }
            // Only port B has pullups
            if (port == PORT_B) {
                WPUB &= ~(1 << pin);
            }
            break;
        case __CHIP_PIN_INPUT_PULLUP:
            *__tris[port] |= (1 << pin);
            if (port == PORT_C) {
                if ((pin == 2) & (__pwmActive & 0x01)) {
                    CCP1CONbits.CCP1M = 0;
                    __pwmActive &= 0xFE;
                    if (__pwmActive == 0) {
                        T2CONbits.TMR2ON = 0;
                    }
                } else if ((pin == 1) && (__pwmActive & 0x02)) {
                    CCP2CONbits.CCP2M = 0;
                    __pwmActive &= 0xFD;
                    if (__pwmActive == 0) {
                        T2CONbits.TMR2ON = 0;
                    }
                }
            }

            an = portPinToAnalogChannel(port, pin);
            if (an != 255) {
                if (an < 8) {
                    ANSEL &= ~(1 << an);
                } else {
                    ANSELH &= ~(1 << (an - 8));
                }
            }
            // Only port B has pullups
            if (port == PORT_B) {
                WPUB |= (1 << pin);
            }
            break;
        case __CHIP_PIN_INPUT_ANALOG:
            *__tris[port] |= (1 << pin);
            if (port == PORT_C) {
                if ((pin == 2) & (__pwmActive & 0x01)) {
                    CCP1CONbits.CCP1M = 0;
                    __pwmActive &= 0xFE;
                    if (__pwmActive == 0) {
                        T2CONbits.TMR2ON = 0;
                    }
                } else if ((pin == 1) && (__pwmActive & 0x02)) {
                    CCP2CONbits.CCP2M = 0;
                    __pwmActive &= 0xFD;
                    if (__pwmActive == 0) {
                        T2CONbits.TMR2ON = 0;
                    }
                }
            }

            an = portPinToAnalogChannel(port, pin);
            if (an != 255) {
                if (an < 8) {
                    ANSEL |= (1 << an);
                } else {
                    ANSELH |= (1 << (an - 8));
                }
            }
            // Only port B has pullups
            if (port == PORT_B) {
                WPUB &= ~(1 << pin);
            }
            break;
    }
}

void __chipSetDigitalValue(uint8_t port, uint8_t pin, uint8_t val) {
    if (val > 0) {
        *__lat[port] |= (1 << pin);
    } else {
        *__lat[port] &= ~(1 << pin);
    }
}

uint8_t __chipGetDigitalValue(uint8_t port, uint8_t pin) {
    return (*__port[port] & (1 << pin)) ? 1 : 0;
}

void __chipSetAnalogValue(uint8_t port, uint8_t pin, int16_t val) {
    if (port == PORT_C) {
        if (pin == 2) { // CCP1
            if ((__pwmActive & 0x01) == 0) {
                __chipSetPinFunction(port, pin, __CHIP_PIN_OUTPUT_ANALOG);
            }
            CCPR1L = val;
        } else if (pin == 1) { // CCP2
            if ((__pwmActive & 0x02) == 0) {
                __chipSetPinFunction(port, pin, __CHIP_PIN_OUTPUT_ANALOG);
            }
            CCPR2L = val;
        } else {
            __chipSetPinFunction(port, pin, __CHIP_PIN_OUTPUT);
            __chipSetDigitalValue(port, pin, val > 127);
        }
    }
}

int16_t __chipGetAnalogValue(uint8_t port, uint8_t pin) {
    uint8_t chan = portPinToAnalogChannel(port, pin);
    int16_t out = 0;
    if (chan == 255) {
        return __chipGetDigitalValue(port, pin) ? 0 : 1023;
    }
    ADCON2 = 0b10101111;
    ADCON1 = 0b00000000;
    ADCON0 = 0b00000001 | (chan << 2);
    ADCON0 |= 0b00000010;
    while (ADCON0 & 0b00000010);
    out = (ADRESH << 8) | ADRESL;
    ADCON0 = 0;
    return out;
}
