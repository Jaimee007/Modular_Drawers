// Shared bus protocol. This single file is used by BOTH firmware projects
// (module/ on the CH32V003 and base_test/ on the ESP32-C3), so the frame
// format and the CRC can never drift apart.
//
// Frame: [START] [DIR] [CMD] [LEN] [DATA...] [CRC8]
//   DIR = 0      -> broadcast
//   DIR = 1..31  -> one module
//   CRC8 covers DIR, CMD, LEN and DATA (not START).
#ifndef BUS_H
#define BUS_H

#include <stdint.h>

#define BUS_START        0xA5
#define BUS_BAUD         115200
#define BUS_MAX_DATA     8
#define BUS_ADDR_BCAST   0
#define BUS_ADDR_MAX     31

// Commands (see docs/02_diseno_hardware.md section 8.3)
enum {
    BUS_CMD_PING          = 0x01,
    BUS_CMD_SET_LED       = 0x02,
    BUS_CMD_GET_STATE     = 0x03,
    BUS_CMD_SENSE_DRIVE   = 0x04,
    BUS_CMD_SENSE_RELEASE = 0x05,
    BUS_CMD_SENSE_QUERY   = 0x06,
    BUS_CMD_IDENTIFY      = 0x07,
};

// CRC-8, polynomial 0x07, init 0x00.
static inline uint8_t bus_crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0;
    while (len--) {
        crc ^= *data++;
        for (uint8_t i = 0; i < 8; i++)
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x07) : (uint8_t)(crc << 1);
    }
    return crc;
}

typedef struct {
    uint8_t dir;
    uint8_t cmd;
    uint8_t len;
    uint8_t data[BUS_MAX_DATA];
} bus_frame_t;

// Serialises a frame into out[]. Returns the number of bytes written,
// or 0 if the payload is too long. out[] must hold BUS_MAX_DATA + 5 bytes.
static inline uint8_t bus_encode(const bus_frame_t *f, uint8_t *out)
{
    if (f->len > BUS_MAX_DATA)
        return 0;
    uint8_t n = 0;
    out[n++] = BUS_START;
    out[n++] = f->dir;
    out[n++] = f->cmd;
    out[n++] = f->len;
    for (uint8_t i = 0; i < f->len; i++)
        out[n++] = f->data[i];
    out[n] = bus_crc8(&out[1], (uint8_t)(n - 1));
    return (uint8_t)(n + 1);
}

// Byte-by-byte receive state machine.
typedef enum { RX_START, RX_DIR, RX_CMD, RX_LEN, RX_DATA, RX_CRC } bus_rx_state_t;

typedef struct {
    bus_rx_state_t state;
    bus_frame_t    frame;
    uint8_t        idx;
} bus_rx_t;

static inline void bus_rx_reset(bus_rx_t *rx)
{
    rx->state = RX_START;
    rx->idx = 0;
}

// Feeds one byte. Returns 1 when a complete, CRC-valid frame is in rx->frame,
// -1 when a frame ended with a bad CRC (e.g. two modules sharing an address),
// and 0 otherwise.
static inline int8_t bus_rx_feed(bus_rx_t *rx, uint8_t b)
{
    switch (rx->state) {
    case RX_START:
        if (b == BUS_START)
            rx->state = RX_DIR;
        break;
    case RX_DIR:
        rx->frame.dir = b;
        rx->state = RX_CMD;
        break;
    case RX_CMD:
        rx->frame.cmd = b;
        rx->state = RX_LEN;
        break;
    case RX_LEN:
        if (b > BUS_MAX_DATA) {
            bus_rx_reset(rx);
            break;
        }
        rx->frame.len = b;
        rx->idx = 0;
        rx->state = b ? RX_DATA : RX_CRC;
        break;
    case RX_DATA:
        rx->frame.data[rx->idx++] = b;
        if (rx->idx >= rx->frame.len)
            rx->state = RX_CRC;
        break;
    case RX_CRC: {
        uint8_t buf[BUS_MAX_DATA + 3];
        buf[0] = rx->frame.dir;
        buf[1] = rx->frame.cmd;
        buf[2] = rx->frame.len;
        for (uint8_t i = 0; i < rx->frame.len; i++)
            buf[3 + i] = rx->frame.data[i];
        uint8_t ok = (bus_crc8(buf, (uint8_t)(3 + rx->frame.len)) == b);
        bus_rx_reset(rx);
        return ok ? 1 : -1;
    }
    }
    return 0;
}

#endif // BUS_H
