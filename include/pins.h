#ifndef PINS_H
#define PINS_H

#include "libraries.h"

/* Pin Configuration */
#define GPIO_SWITCH_CHANNEL_A ((uint8_t) 17)
#define GPIO_SWITCH_CHANNEL_B ((uint8_t) 27)
#define GPIO_SWITCH_CHANNEL_C ((uint8_t) 22)

#define GPIO_SDO ((uint8_t) 9)  // MISO
#define GPIO_SDI ((uint8_t) 10) // MOSI
#define GPIO_SCK ((uint8_t) 11) // SCLK

#define GPIO_UD  ((uint8_t) 5)

#define GPIO_IRQ ((uint8_t) 25)

#define GPIO_CS1 ((uint8_t) 6)
#define GPIO_CS2 ((uint8_t) 13)
#define GPIO_CS3 ((uint8_t) 19)
#define GPIO_CS4 ((uint8_t) 26)
#define GPIO_CS5 ((uint8_t) 20)
#define GPIO_CS6 ((uint8_t) 21)
#define GPIO_CS7 ((uint8_t) 23)
#define GPIO_CS8 ((uint8_t) 24)

// ADC SPI chip select on CE1 (BCM GPIO 7, header pin 26)
#define SPI_CS ((uint8_t) RPI_GPIO_P1_26)

#endif
