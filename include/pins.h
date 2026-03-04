#ifndef PINS_H
#define PINS_H 1

#include "libraries.h"

/* Pin Configuration */
constexpr uint8_t GPIO_SWITCH_CHANNEL_A = 17;
constexpr uint8_t GPIO_SWITCH_CHANNEL_B = 27;
constexpr uint8_t GPIO_SWITCH_CHANNEL_C = 22;

constexpr uint8_t GPIO_SDO = 9;  // MISO
constexpr uint8_t GPIO_SDI = 10; // MOSI
constexpr uint8_t GPIO_SCK = 11; // SCLK

constexpr uint8_t GPIO_UD  = 5;

constexpr uint8_t GPIO_IRQ = 25;

constexpr uint8_t GPIO_CS1 = 6;
constexpr uint8_t GPIO_CS2 = 13;
constexpr uint8_t GPIO_CS3 = 19;
constexpr uint8_t GPIO_CS4 = 26;
constexpr uint8_t GPIO_CS5 = 20;
constexpr uint8_t GPIO_CS6 = 21;
constexpr uint8_t GPIO_CS7 = 23;
constexpr uint8_t GPIO_CS8 = 24;

// SPI Chip Select (1)
constexpr uint8_t SPI_CS = 1;

#endif

