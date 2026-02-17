/*
 * MCP3564 ADC Program for Raspberry Pi
 * Requirements: libbcm2835 (sudo apt-get install libbcm2835-dev)
 * Compilation: gcc -o mcp3564 mcp3564.c -lbcm2835
 * Run with: sudo ./mcp3564
 */


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <bcm2835.h>
#include <stdbool.h>  // for bool type

#define GPIO_SDI ((uint8_t)10) // MOSI 
#define GPIO_SDO ((uint8_t)9)  // MISO 
#define GPIO_SCK ((uint8_t)11) // SCLK 
#define SPI_CS   ((uint8_t)1)  // Chip Select 

// MCP3564 register addresses
#define REG_ADCCONFIG   0x01
#define REG_ADCSTATUS   0x02
#define REG_ADCDATA     0x03
#define REG_SCAN        0x04
#define REG_CONFIG0     0x05
#define REG_CONFIG1     0x06
#define REG_CONFIG2     0x07
#define REG_CONFIG3     0x08


// SPI helper functions (manual CS control)
void cs_low() {
    bcm2835_gpio_write(SPI_CS, LOW);   // CS = 0 -> device selected
}

void cs_high() {
    bcm2835_gpio_write(SPI_CS, HIGH);  // CS = 1 -> deselected
}

// Performs SPI transfer with CS control
void spi_transfer(uint8_t *tx, uint8_t *rx, int len) {
     if (!tx || !rx || len <= 0) return; // check null
    cs_low();
    bcm2835_spi_transfernb((char*)tx, (char*)rx, len);
    cs_high();
}


// Functions for MCP3564 communication

// Write to register
void write_reg(uint8_t addr, uint8_t value) {
    uint8_t cmd = (addr << 1) & 0x7E;   // R/W=0, address shift, bit0=0
    uint8_t tx[2] = {cmd, value};
    uint8_t rx[2];
    spi_transfer(tx, rx, 2);
}

// Read single register
uint8_t read_reg(uint8_t addr) {
    uint8_t cmd = 0x80 | ((addr << 1) & 0x7E); // R/W=1 (read mode)
    uint8_t tx[2] = {cmd, 0x00};
    uint8_t rx[2];
    spi_transfer(tx, rx, 2);
    return rx[1]; // second byte is the register value
}

// Read 24-bit data from ADCDATA register
int32_t read_data() {
    uint8_t cmd = 0x86;   // read ADCDATA (address 0x03)
    uint8_t tx[4] = {cmd, 0x00, 0x00, 0x00};
    uint8_t rx[4];
    spi_transfer(tx, rx, 4);
    int32_t val = (rx[1] << 16) | (rx[2] << 8) | rx[3];
    // Sign extension for 24-bits, probably not needed for us ???
    
    //if (val & 0x800000)
    //     val -= 0x1000000;
    return val;
}


bool init_raspberry_pi_spi(uint8_t cs_pin, uint32_t clock_divider) {
    // Initialize bcm2835 library
    if (!bcm2835_init()) {
        fprintf(stderr, "bcm2835 initialization error\n");
        return false;
    }

    // SPI Configuration
    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0); // Set SPI mode to 0
    
    // Set SPI clock speed
    bcm2835_spi_setClockDivider(clock_divider);
    
    // Disable hardware CS - we will control it manually
    bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);
    
    // Configure CS pin as output
    bcm2835_gpio_fsel(cs_pin, BCM2835_GPIO_FSEL_OUTP);
    
    // Set CS to high state (inactive)
    bcm2835_gpio_write(cs_pin, HIGH);
    
    return true;
}

void cleanup_raspberry_pi_spi() {
    bcm2835_spi_end();
    bcm2835_close();
}


// MCP3564 Initialization
void init_mcp3564() {
    // 1. Enter standby mode (stop conversion during configuration)
    write_reg(REG_ADCCONFIG, 0x00);
    bcm2835_delay(1); // short wait

    // 2. Channel configuration: all as single-ended, gain 1
    //    (SIGN=0, GAIN=000)
    write_reg(REG_CONFIG0, 0x00); // channels 0 and 1
    write_reg(REG_CONFIG1, 0x00); // channels 2 and 3
    write_reg(REG_CONFIG2, 0x00); // channels 4 and 5
    write_reg(REG_CONFIG3, 0x00); // channels 6 and 7

    // 3. Enable scanning for all 8 channels
    write_reg(REG_SCAN, 0xFF); // bits 0-7 = 1

    // 4. Set continuous conversion mode, gain=1, speed 102.4 kHz
    //    Value 0x85: MODE=10 (continuous), GAIN=000, SPEED=101
    write_reg(REG_ADCCONFIG, 0x85);

}


// Main function - reads current values from 8 microphones into an array
// tu trzeba dodac time out jak zawiesi sie adc (napiszę go pozniej jak bedziemy mieli płytke i sie okaze że jest potrzebny)
void read_all_channels(float values[8]) {
    // Helper arrays
    int32_t raw[8] = {0};
    int received[8] = {0};
    int samples_collected = 0;

    // Collect samples until all 8 channels are received
    while (samples_collected < 8) {
        uint8_t status = read_reg(REG_ADCSTATUS);
        if (status & 0x80) { // DR (Data Ready) bit = 1
            uint8_t channel = (status >> 4) & 0x07; // Extract Channel ID
            int32_t data = read_data();

            // If we don't have a sample for this channel yet, save it
            if (!received[channel]) {
                raw[channel] = data;
                received[channel] = 1;
                samples_collected++;
            }
            // If already received, ignore (but data was read,
            // so DR flag is cleared and conversions continue)
        }
    }

    // Conversion of raw values to voltage (optional)
    // Assume Vref = 3.3 V (if AVDD = 3.3 V)
    for (int i = 0; i < 8; i++) {
        // For single-ended mode range 0..16777215 corresponds to 0..Vref
        values[i] = (float)raw[i] * 3.3f / 16777215.0f;
    }
}

/*
int main() {
    // Hardware initialization (need to check if 8 channels work without errors)
    if (!init_raspberry_pi_spi(SPI_CS, BCM2835_SPI_CLOCK_DIVIDER_16)) {
        return 1;
    }

    // MCP3564 Initialization
    init_mcp3564();

    // Reading microphone values
    float channel_values[8];
    read_all_channels(channel_values);

    // Displaying results
    printf("Read values from microphones (voltages):\n");
    for (int i = 0; i < 8; i++) {
        printf("CH%d: %.3f V\n", i, channel_values[i]);
    }

    // Cleanup
    cleanup_raspberry_pi_spi();
    return 0;
}*/
