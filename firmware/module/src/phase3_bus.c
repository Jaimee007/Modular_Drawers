// phase3_bus: bidirectional half-duplex bus, module side.
// Env: phase3_bus (pio run -e phase3_bus -t upload)
// Doc: docs/07_pruebas_banco.md#phase3_bus
//
// The module listens on DATA and answers PING and GET_STATE when the frame is
// addressed to its own address (or to the broadcast address, which never
// answers, to avoid collisions).
//
// Wiring (see docs/06_montaje_protoboard.md section 2):
//   PD5 --[1 kohm]-- DATA     (DATA also goes to the base RX/TX and a single
//                              4.7 kohm pull-up to 3V3, on the base)
//   address jumpers PC0..PC4 to GND (weights 1,2,4,8,16): closed = bit set
//   limit switch on PD6 (closed to GND = pressed), optional here
//   common GND with the base
//
// How the half-duplex line works:
//   - USART1 runs with HDSEL: TX and RX share PD5.
//   - PD5 is alternate-function OPEN-DRAIN. The module only ever pulls the line
//     low, and the single pull-up on the base brings it high.
//   - Because TX and RX are tied, every transmitted byte is also received. That
//     echo is flushed before the next frame is parsed.
//
// Debug output goes over printf (SWIO). Read it with: minichlink -T
#include "ch32fun.h"
#include <stdio.h>

#include "bus.h"

#define REPLY_DELAY_US 100 // let the base release the line before answering

static uint8_t my_addr;
static uint32_t frames_ok, frames_bad, frames_mine;
static uint8_t drawer_open; // 1 = open (limit switch released), reported by GET_STATE

static void gpio_setup(void)
{
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1;

    // PC0..PC4 address jumpers, PD6 limit switch: input with pull-up.
    for (int pin = 0; pin < 5; pin++) {
        GPIOC->CFGLR = (GPIOC->CFGLR & ~(0xF << (4 * pin))) | (GPIO_CNF_IN_PUPD << (4 * pin));
        GPIOC->BSHR = (1u << pin); // ODR = 1 selects the pull-up
    }
    GPIOD->CFGLR = (GPIOD->CFGLR & ~(0xF << (4 * 6))) | (GPIO_CNF_IN_PUPD << (4 * 6));
    GPIOD->BSHR = (1u << 6);
}

static uint8_t read_address(void)
{
    uint8_t a = 0;
    for (int pin = 0; pin < 5; pin++)
        if (!(GPIOC->INDR & (1u << pin))) // closed to GND reads 0
            a |= (uint8_t)(1u << pin);
    return a;
}

static void uart_setup(void)
{
    // PD5 as alternate-function open-drain, 10 MHz: USART1 TX/RX in half-duplex.
    GPIOD->CFGLR = (GPIOD->CFGLR & ~(0xF << (4 * 5))) |
                   ((GPIO_Speed_10MHz | GPIO_CNF_OUT_OD_AF) << (4 * 5));

    USART1->CTLR1 = USART_WordLength_8b | USART_Parity_No | USART_Mode_Tx | USART_Mode_Rx;
    USART1->CTLR2 = USART_StopBits_1;
    USART1->CTLR3 = USART_HardwareFlowControl_None | USART_CTLR3_HDSEL;

    USART1->BRR = (FUNCONF_SYSTEM_CORE_CLOCK + BUS_BAUD / 2) / BUS_BAUD;
    USART1->CTLR1 |= CTLR1_UE_Set;
}

// Reads and discards everything pending in the receiver (echo or noise).
static void rx_flush(void)
{
    while (USART1->STATR & (USART_STATR_RXNE | USART_STATR_ORE))
        (void)USART1->DATAR; // reading DATAR also clears ORE after reading STATR
}

static void send_frame(const bus_frame_t *f)
{
    uint8_t buf[BUS_MAX_DATA + 5];
    uint8_t n = bus_encode(f, buf);
    for (uint8_t i = 0; i < n; i++) {
        while (!(USART1->STATR & USART_STATR_TXE))
            ;
        USART1->DATAR = buf[i];
    }
    while (!(USART1->STATR & USART_STATR_TC))
        ;
    // In half-duplex the transmitted bytes come back on RX: drop them.
    Delay_Us(50);
    rx_flush();
}

static void handle_frame(const bus_frame_t *rx)
{
    // Broadcast is received but never answered: every module would answer at once.
    if (rx->dir != my_addr)
        return;
    frames_mine++;

    bus_frame_t tx = {};
    tx.dir = my_addr;
    tx.cmd = rx->cmd;

    switch (rx->cmd) {
    case BUS_CMD_PING:
        // Echo the payload so the base can check the round trip.
        tx.len = rx->len;
        for (uint8_t i = 0; i < rx->len; i++)
            tx.data[i] = rx->data[i];
        break;
    case BUS_CMD_GET_STATE:
        tx.len = 2;
        tx.data[0] = drawer_open;
        tx.data[1] = (uint8_t)(frames_ok & 0xFF);
        break;
    default:
        return; // unknown command: stay silent
    }

    Delay_Us(REPLY_DELAY_US);
    send_frame(&tx);
}

int main(void)
{
    SystemInit();
    gpio_setup();
    Delay_Ms(10); // let the pull-ups settle before reading the address
    my_addr = read_address();
    uart_setup();

    printf("phase3_bus up, address=%d%s\n", my_addr, my_addr == 0 ? " (unconfigured)" : "");

    bus_rx_t rx;
    bus_rx_reset(&rx);
    uint32_t last_report = SysTick->CNT;
    uint32_t last_byte = SysTick->CNT;

    while (1) {
        // The limit switch is closed to GND when the drawer is closed.
        drawer_open = (GPIOD->INDR & (1u << 6)) ? 1 : 0;

        if (USART1->STATR & USART_STATR_ORE)
            (void)USART1->DATAR; // overrun: drop it

        if (USART1->STATR & USART_STATR_RXNE) {
            uint8_t b = (uint8_t)USART1->DATAR;
            last_byte = SysTick->CNT;
            int8_t r = bus_rx_feed(&rx, b);
            if (r == 1) {
                frames_ok++;
                handle_frame(&rx.frame);
            } else if (r == -1) {
                frames_bad++;
            }
        } else if ((SysTick->CNT - last_byte) > (uint32_t)(DELAY_MS_TIME * 5)) {
            // A frame that stalls half-way would swallow the next START: resync.
            bus_rx_reset(&rx);
            last_byte = SysTick->CNT;
        }

        // Once a second, report counters (the printf is slow without a terminal).
        if ((SysTick->CNT - last_report) > (uint32_t)(DELAY_MS_TIME * 1000)) {
            last_report = SysTick->CNT;
            printf("ok=%lu bad=%lu mine=%lu open=%d\n", (unsigned long)frames_ok,
                   (unsigned long)frames_bad, (unsigned long)frames_mine, drawer_open);
        }
    }
}
