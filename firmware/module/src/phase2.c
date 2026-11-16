// Phase 2 bench test: module peripherals, one by one, no bus and no ESP32.
// Everything is reported with printf() over SWIO. Read it with: minichlink -T
//
// What it checks (pins from docs/02_diseno_hardware.md section 2.1):
//   - Address jumpers PC0..PC4 (weights 1,2,4,8,16): closed to GND = bit set.
//   - Pairing jumpers PC5 (JP7, left) and PD4 (JP8, down): closed to GND = set.
//   - Limit switch on PD6, closed to GND = pressed = 0, 25 ms debounce.
//   - The four SENSE pins PA1 (L), PA2 (R), PD2 (U), PD3 (D), printed every 200 ms.
//
// Not covered here: the RGB LED (needs the 2N7002 stage) and the bus.
//
// Note: with FUNCONF_USE_DEBUGPRINTF, printf() waits up to ~200 ms for a host
// to read it when no minichlink terminal is attached, so keep one open.
#include "ch32fun.h"
#include <stdio.h>

#define DEBOUNCE_MS 25
#define SENSE_PERIOD_MS 200

static const uint8_t ADDR_PINS[5] = {PC0, PC1, PC2, PC3, PC4}; // weights 1,2,4,8,16
static const uint8_t SENSE_PINS[4] = {PA1, PA2, PD2, PD3};     // L, R, U, D
static const char SENSE_NAMES[4] = {'L', 'R', 'U', 'D'};

#define PIN_JP7_LEFT PC5
#define PIN_JP8_DOWN PD4
#define PIN_SWITCH PD6

static void pin_input_pullup(uint8_t pin)
{
    funPinMode(pin, GPIO_CNF_IN_PUPD);
    funDigitalWrite(pin, FUN_HIGH); // ODR = 1 selects the pull-up
}

// Closed to GND reads 0, so "closed" is the inverted level.
static int closed(uint8_t pin)
{
    return !funDigitalRead(pin);
}

static uint8_t read_address(void)
{
    uint8_t addr = 0;
    for (uint8_t i = 0; i < 5; i++)
        if (closed(ADDR_PINS[i]))
            addr |= (uint8_t)(1u << i);
    return addr;
}

static void print_sense(void)
{
    printf("SENSE");
    for (uint8_t i = 0; i < 4; i++)
        printf(" %c=%d", SENSE_NAMES[i], funDigitalRead(SENSE_PINS[i]));
    printf("\n");
}

int main(void)
{
    SystemInit();
    funGpioInitAll();

    for (uint8_t i = 0; i < 5; i++)
        pin_input_pullup(ADDR_PINS[i]);
    for (uint8_t i = 0; i < 4; i++)
        pin_input_pullup(SENSE_PINS[i]);
    pin_input_pullup(PIN_JP7_LEFT);
    pin_input_pullup(PIN_JP8_DOWN);
    pin_input_pullup(PIN_SWITCH);

    Delay_Ms(10); // let the pull-ups settle before the one-time read

    uint8_t addr = read_address();
    printf("Phase 2 test\n");
    printf("address=%d%s pair_left=%d pair_down=%d\n", addr,
           addr == 0 ? " (unconfigured)" : "", closed(PIN_JP7_LEFT),
           closed(PIN_JP8_DOWN));

    int stable = closed(PIN_SWITCH); // 1 = pressed
    int candidate = stable;
    uint8_t count = 0;
    uint16_t sense_ms = 0;
    uint32_t events = 0;

    printf("switch=%s\n", stable ? "CLOSED" : "OPEN");

    while (1) {
        // Debounce: the state only changes once it has been stable for DEBOUNCE_MS.
        int raw = closed(PIN_SWITCH);
        if (raw == stable) {
            count = 0;
        } else {
            if (raw != candidate) {
                candidate = raw;
                count = 0;
            }
            if (++count >= DEBOUNCE_MS) {
                stable = raw;
                count = 0;
                events++;
                printf("switch=%s events=%lu\n", stable ? "CLOSED" : "OPEN",
                       (unsigned long)events);
            }
        }

        if (++sense_ms >= SENSE_PERIOD_MS) {
            sense_ms = 0;
            print_sense();
        }

        Delay_Ms(1);
    }
}
