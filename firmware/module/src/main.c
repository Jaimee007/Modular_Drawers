// Phase 3 bench test, module side: send a PING frame every second on the bus
// line (PD5 = USART1 TX). No LED is needed on the module: the base prints and
// lights its own LED when it receives a valid frame.
//
// Bus test wiring (see firmware/base_test/src/main.cpp for the base side):
//   module PD5 --[1 kohm]--+-- DATA --+-- base RX pin
//                          |          +-- 4.7 kohm pull-up to 3V3
//   module GND ------------ common GND with the base
//
// This first test only transmits, so the line is driven push-pull. The final
// half-duplex, open-drain mode (HDSEL) comes in the next step.
#include "ch32fun.h"
#include <stdio.h>

#include "bus.h"

#define MODULE_ADDR 1

static void uart_setup(void)
{
    // Enable GPIOD and USART1.
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1;

    // PD5 as alternate-function push-pull, 10 MHz (USART1 TX).
    GPIOD->CFGLR = (GPIOD->CFGLR & ~(0xF << (4 * 5))) |
                   ((GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF) << (4 * 5));

    // 8N1, transmitter only.
    USART1->CTLR1 = USART_WordLength_8b | USART_Parity_No | USART_Mode_Tx;
    USART1->CTLR2 = USART_StopBits_1;
    USART1->CTLR3 = USART_HardwareFlowControl_None;

    USART1->BRR = (FUNCONF_SYSTEM_CORE_CLOCK + BUS_BAUD / 2) / BUS_BAUD;
    USART1->CTLR1 |= CTLR1_UE_Set;
}

static void uart_send(uint8_t b)
{
    while (!(USART1->STATR & USART_STATR_TXE))
        ;
    USART1->DATAR = b;
}

static void send_frame(const bus_frame_t *f)
{
    uint8_t buf[BUS_MAX_DATA + 5];
    uint8_t n = bus_encode(f, buf);
    for (uint8_t i = 0; i < n; i++)
        uart_send(buf[i]);
    // Wait for the last byte to leave the shifter.
    while (!(USART1->STATR & USART_STATR_TC))
        ;
}

int main(void)
{
    SystemInit();
    uart_setup();

    uint8_t seq = 0;
    while (1) {
        bus_frame_t f = {};
        f.dir = MODULE_ADDR;
        f.cmd = BUS_CMD_PING;
        f.len = 1;
        f.data[0] = seq++; // sequence number, so lost frames are visible on the base
        send_frame(&f);
        Delay_Ms(1000);
    }
}
