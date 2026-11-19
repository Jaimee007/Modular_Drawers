// base_bus: bidirectional bus, base side (ESP32-C3 SuperMini).
// Env: base_bus (pio run -e base_bus -t upload)
// Doc: docs/07_pruebas_banco.md#base_bus
//
// Serial menu (115200, end each line with Enter):
//   ping N            one PING to module N, shows the round trip
//   stress N COUNT    COUNT PINGs in a row to module N, counts errors
//   state N           GET_STATE to module N (drawer open/closed)
//   scan              PING to addresses 1..31, lists the ones that answer
//   help
//
// Wiring (docs/06_montaje_protoboard.md section 2):
//   GPIO20 (RX) --> DATA
//   GPIO21 (TX) --|<|-- DATA   (Schottky, band toward GPIO21)
//   DATA --[4.7 kohm]--> 3V3    (the single pull-up of the system)
//   module PD5 --[1 kohm]--> DATA, common GND
//
// The TX pin drives the line through the diode, so it can only pull DATA low.
// RX sees what the base transmits too (echo): it is flushed before waiting for
// the reply.
#include <Arduino.h>

#include "bus.h"

#define BUS_RX_PIN 20
#define BUS_TX_PIN 21
#define REPLY_TIMEOUT_MS 20

static HardwareSerial &busSerial = Serial1;

// Result of one request/reply exchange.
enum Result { RES_OK, RES_TIMEOUT, RES_BAD_CRC, RES_MISMATCH };

static void flushRx()
{
    while (busSerial.available())
        busSerial.read();
}

static void sendFrame(const bus_frame_t &f)
{
    uint8_t buf[BUS_MAX_DATA + 5];
    uint8_t n = bus_encode(&f, buf);
    flushRx();
    busSerial.write(buf, n);
    busSerial.flush();   // wait until the last byte left the UART
    // The bus is half-duplex, so RX also receives our own n bytes. Drop exactly
    // n bytes instead of waiting a fixed time: a fixed wait can end before the
    // echo finishes (and the tail is then read as a reply) or after the module
    // has started answering (and the start of the reply is lost).
    uint32_t t0 = micros();
    uint8_t dropped = 0;
    while (dropped < n && (micros() - t0) < 3000) { // 3 ms >> one frame time
        if (busSerial.available()) {
            busSerial.read();
            dropped++;
        }
    }
}

// Sends a frame and waits for the module's reply. On RES_OK, reply holds it.
static Result exchange(const bus_frame_t &req, bus_frame_t &reply)
{
    sendFrame(req);

    bus_rx_t rx;
    bus_rx_reset(&rx);
    uint32_t start = millis();
    while (millis() - start < REPLY_TIMEOUT_MS) {
        while (busSerial.available()) {
            int8_t r = bus_rx_feed(&rx, (uint8_t)busSerial.read());
            if (r == 1) {
                reply = rx.frame;
                if (reply.dir != req.dir || reply.cmd != req.cmd)
                    return RES_MISMATCH;
                return RES_OK;
            }
            if (r == -1)
                return RES_BAD_CRC;
        }
    }
    return RES_TIMEOUT;
}

static const char *resultName(Result r)
{
    switch (r) {
    case RES_OK: return "ok";
    case RES_TIMEOUT: return "timeout";
    case RES_BAD_CRC: return "bad crc";
    default: return "mismatch";
    }
}

static bool doPing(uint8_t addr, uint8_t seq, Result *out = nullptr)
{
    bus_frame_t req = {};
    req.dir = addr;
    req.cmd = BUS_CMD_PING;
    req.len = 1;
    req.data[0] = seq;
    bus_frame_t reply = {};
    Result r = exchange(req, reply);
    if (r == RES_OK && (reply.len != 1 || reply.data[0] != seq))
        r = RES_MISMATCH; // the module must echo the payload
    if (out) *out = r;
    return r == RES_OK;
}

static void cmdPing(uint8_t addr)
{
    uint32_t t0 = micros();
    Result r;
    bool ok = doPing(addr, 0x5A, &r);
    uint32_t dt = micros() - t0;
    Serial.printf("ping %u: %s (%lu us)\n", addr, ok ? "reply ok" : resultName(r),
                  (unsigned long)dt);
}

static void cmdStress(uint8_t addr, uint32_t count)
{
    uint32_t ok = 0, timeout = 0, crc = 0, mismatch = 0;
    uint32_t t0 = millis();
    for (uint32_t i = 0; i < count; i++) {
        Result r;
        if (doPing(addr, (uint8_t)i, &r)) ok++;
        else if (r == RES_TIMEOUT) timeout++;
        else if (r == RES_BAD_CRC) crc++;
        else mismatch++;
        if ((i & 0x3F) == 0x3F) yield();
    }
    uint32_t dt = millis() - t0;
    Serial.printf("stress %u x%lu: ok=%lu timeout=%lu crc=%lu mismatch=%lu (%lu ms) -> %s\n",
                  addr, (unsigned long)count, (unsigned long)ok, (unsigned long)timeout,
                  (unsigned long)crc, (unsigned long)mismatch, (unsigned long)dt,
                  ok == count ? "PASS (0 errors)" : "ERRORS");
}

static void cmdState(uint8_t addr)
{
    bus_frame_t req = {};
    req.dir = addr;
    req.cmd = BUS_CMD_GET_STATE;
    bus_frame_t reply = {};
    Result r = exchange(req, reply);
    if (r != RES_OK || reply.len < 2) {
        Serial.printf("state %u: %s", addr, r == RES_OK ? "short reply" : resultName(r));
        if (r == RES_OK) {
            // Show what actually arrived, to tell a firmware mismatch from a bug.
            Serial.printf(" (dir=%u cmd=0x%02X len=%u)", reply.dir, reply.cmd, reply.len);
            for (uint8_t i = 0; i < reply.len && i < BUS_MAX_DATA; i++)
                Serial.printf(" %02X", reply.data[i]);
        }
        Serial.println();
        return;
    }
    Serial.printf("state %u: drawer %s, module frames ok (low byte) = %u\n", addr,
                  reply.data[0] ? "open" : "closed", reply.data[1]);
}

static void cmdScan()
{
    Serial.print("scan: ");
    bool any = false;
    for (uint8_t a = 1; a <= BUS_ADDR_MAX; a++) {
        if (doPing(a, a)) {
            Serial.printf("%u ", a);
            any = true;
        }
    }
    Serial.println(any ? "answered" : "no module answered");
}

static void printHelp()
{
    Serial.println("commands: ping N | stress N COUNT | state N | scan | help");
}

static void handleLine(String line)
{
    line.trim();
    line.toLowerCase();
    if (line.length() == 0) return;

    unsigned a = 0, n = 0;
    if (line == "help") printHelp();
    else if (line == "scan") cmdScan();
    else if (sscanf(line.c_str(), "ping %u", &a) == 1 && a >= 1 && a <= BUS_ADDR_MAX) cmdPing(a);
    else if (sscanf(line.c_str(), "stress %u %u", &a, &n) == 2 && a >= 1 && a <= BUS_ADDR_MAX &&
             n >= 1 && n <= 100000) cmdStress(a, n);
    else if (sscanf(line.c_str(), "state %u", &a) == 1 && a >= 1 && a <= BUS_ADDR_MAX) cmdState(a);
    else Serial.printf("unknown or invalid command '%s' (type help)\n", line.c_str());
}

void setup()
{
    Serial.begin(115200);
    delay(1500); // let the USB CDC port enumerate
    busSerial.begin(BUS_BAUD, SERIAL_8N1, BUS_RX_PIN, BUS_TX_PIN);
    Serial.printf("base_bus up: RX=GPIO%d TX=GPIO%d at %d baud\n", BUS_RX_PIN, BUS_TX_PIN,
                  BUS_BAUD);
    printHelp();
}

void loop()
{
    static String line;
    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == '\n' || c == '\r') {
            handleLine(line);
            line = "";
        } else if (line.length() < 40) {
            line += c;
        }
    }
}
