#ifndef _FUNCONFIG_H
#define _FUNCONFIG_H

// The clock (internal oscillator + PLL, 48 MHz) is set by the PlatformIO
// platform, so it is not repeated here. Full list of options: ch32fun/ch32fun.h

// printf() goes out over the same SWIO wire used to program the chip, because
// the USART is reserved for the bus. Read it with: minichlink -T
#define FUNCONF_USE_DEBUGPRINTF 1

#endif
