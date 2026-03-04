#ifndef ADC_H
#define ADC_H 1

#include "libraries.h"
#include "pins.h"

// MCP3564 register addresses
constexpr uint8_t REG_ADCCONFIG = 1;
constexpr uint8_t REG_ADCSTATUS = 2;
constexpr uint8_t REG_ADCDATA   = 3;
constexpr uint8_t REG_SCAN      = 4;
constexpr uint8_t REG_CONFIG0   = 5;
constexpr uint8_t REG_CONFIG1   = 6;
constexpr uint8_t REG_CONFIG2   = 7;
constexpr uint8_t REG_CONFIG3   = 8;

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
