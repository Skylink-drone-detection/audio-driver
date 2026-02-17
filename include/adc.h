/*
 * MCP3564 ADC Program for Raspberry Pi
 * Requirements: libbcm2835 (sudo apt-get install libbcm2835-dev)
 * Compilation: gcc -o mcp3564 mcp3564.c -lbcm2835
 * Run with: sudo ./mcp3564
 */
#ifndef ADC_H
#define ADC_H

#include "libraries.h"
#include "pins.h"

// MCP3564 register addresses
#define REG_ADCCONFIG   (0x01)
#define REG_ADCSTATUS   (0x02)
#define REG_ADCDATA     (0x03)
#define REG_SCAN        (0x04)
#define REG_CONFIG0     (0x05)
#define REG_CONFIG1     (0x06)
#define REG_CONFIG2     (0x07)
#define REG_CONFIG3     (0x08)

void cs_low(void);
void cs_high(void);
void spi_transfer(uint8_t *tx, uint8_t *rx, int len);
void write_reg(uint8_t addr, uint8_t value);
uint8_t read_reg(uint8_t addr);
int32_t read_data(void);
bool init_raspberry_pi_spi(uint8_t cs_pin, uint32_t clock_divider);
void cleanup_raspberry_pi_spi(void);
void init_mcp3564(void);
void read_all_channels(float values[8]);

#endif
