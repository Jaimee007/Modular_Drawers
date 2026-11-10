// Phase 3 bench test, base side: listen on the bus line, validate each frame
// with the shared bus.h, print it over USB and blink the board LED.
//
// Wiring (see firmware/module/src/main.c for the module side):
//   DATA line --> BUS_RX_PIN
//   DATA line --[4.7 kohm]--> 3V3          (the single pull-up of the system)
//   module GND --- base GND                (common ground is mandatory)
// This first test only receives, so BUS_TX_PIN is left unconnected.
#include <Arduino.h>

#include "bus.h"

// SuperMini pins: GPIO20 / GPIO21 are the default UART0 RX / TX. Avoid GPIO5
// and GPIO6: they are boot-strapping pins on the ESP32-C3.
#define BUS_RX_PIN 20
#define BUS_TX_PIN 21
// The SuperMini's on-board LED is on GPIO8 (active low on most boards).
#define LED_PIN 8

static HardwareSerial &busSerial = Serial1;
static bus_rx_t rx;
static uint32_t frames_ok, frames_bad;
static uint32_t raw_bytes;          // every byte seen on the line, valid frame or not
static uint8_t last_raw[8];         // most recent raw bytes, for diagnosis
static uint8_t last_raw_n;

// Round-trips a PING frame through the encoder and the receive state machine.
static bool selfTest()
{
    bus_frame_t tx = {};
    tx.dir = 5;
    tx.cmd = BUS_CMD_PING;
    tx.len = 0;

    uint8_t buf[BUS_MAX_DATA + 5];
    uint8_t n = bus_encode(&tx, buf);

    bus_rx_t t;
    bus_rx_reset(&t);
    int8_t r = 0;
    for (uint8_t i = 0; i < n; i++)
        r = bus_rx_feed(&t, buf[i]);
    if (r != 1 || t.frame.dir != 5 || t.frame.cmd != BUS_CMD_PING)
        return false;

    // A corrupted byte must be reported as a CRC error.
    buf[1] ^= 0x01;
    bus_rx_reset(&t);
    for (uint8_t i = 0; i < n; i++)
        r = bus_rx_feed(&t, buf[i]);
    return r == -1;
}

void setup()
{
    Serial.begin(115200);
    delay(1500); // give the USB CDC port time to enumerate
    Serial.printf("base_test up, bus.h self-test: %s\n", selfTest() ? "PASS" : "FAIL");

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // off (active low)

    busSerial.begin(BUS_BAUD, SERIAL_8N1, BUS_RX_PIN, BUS_TX_PIN);
    bus_rx_reset(&rx);
    Serial.printf("listening on GPIO%d at %d baud\n", BUS_RX_PIN, BUS_BAUD);
}

void loop()
{
    // Once a second, report what the RX line is doing. This separates
    // "nothing arrives" (raw=0, idle level stuck) from "bytes arrive but are
    // wrong" (raw>0 but no frames).
    static uint32_t last_report;
    if (millis() - last_report >= 1000) {
        last_report = millis();
        Serial.printf("[diag] raw bytes=%lu, RX pin level=%d, last:", (unsigned long)raw_bytes,
                      digitalRead(BUS_RX_PIN));
        for (uint8_t i = 0; i < last_raw_n; i++)
            Serial.printf(" %02X", last_raw[i]);
        Serial.println();
    }

    while (busSerial.available()) {
        uint8_t b = (uint8_t)busSerial.read();
        raw_bytes++;
        if (last_raw_n < sizeof(last_raw))
            last_raw[last_raw_n++] = b;
        else {
            memmove(last_raw, last_raw + 1, sizeof(last_raw) - 1);
            last_raw[sizeof(last_raw) - 1] = b;
        }
        int8_t r = bus_rx_feed(&rx, b);

        if (r == 1) {
            frames_ok++;
            Serial.printf("[ack] frame ok: dir=%u cmd=0x%02X len=%u data0=%u (ok=%lu bad=%lu)\n",
                          rx.frame.dir, rx.frame.cmd, rx.frame.len,
                          rx.frame.len ? rx.frame.data[0] : 0,
                          (unsigned long)frames_ok, (unsigned long)frames_bad);
            digitalWrite(LED_PIN, LOW); // on
            delay(80);
            digitalWrite(LED_PIN, HIGH);
        } else if (r == -1) {
            frames_bad++;
            Serial.printf("[err] bad CRC (ok=%lu bad=%lu)\n", (unsigned long)frames_ok,
                          (unsigned long)frames_bad);
        }
    }
}
