#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define HIGH 1
#define LOW 0

#define BCM2835_SPI_CLOCK_DIVIDER_16 16U

bool hardware_init(void);
void hardware_cleanup(void);

bool gpio_request_output(uint8_t gpio, uint8_t initial_value);
bool gpio_write_output(uint8_t gpio, uint8_t value);
void gpio_release(uint8_t gpio);

void hardware_delay_ms(uint32_t milliseconds);
void hardware_delay_us(uint32_t microseconds);

bool spi_open_for_cs(uint8_t cs_gpio, uint32_t clock_divider);
bool spi_transfer_bytes(const uint8_t *tx, uint8_t *rx, size_t len);
void spi_close(void);

#endif
