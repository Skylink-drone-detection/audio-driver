#include "switch_logic.h"

/**
 * @brief Initializes the BCM2835 library and GPIO pins.
 * @return true on success.
 */
bool Audio_Init(AudioRouting_t *ctx) {
    if (!bcm2835_init()) {
        fprintf(stderr, "Cannot initialize BCM2835!\n");
        return false;
    }

    // Ustaw piny jako OUTPUT
    bcm2835_gpio_fsel(GPIO_SWITCH_CHANNEL_A, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(GPIO_SWITCH_CHANNEL_B, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(GPIO_SWITCH_CHANNEL_C, BCM2835_GPIO_FSEL_OUTP);

    // Ustaw initial LOW
    bcm2835_gpio_set(GPIO_SWITCH_CHANNEL_A);
    bcm2835_gpio_clr(GPIO_SWITCH_CHANNEL_B);
    bcm2835_gpio_clr(GPIO_SWITCH_CHANNEL_C);
    
    if (ctx) ctx->initialized = true;
    return true;
}

/**
 * @brief Sets the signal path for all channels atomically.
 */
void Audio_SetFilters(AudioRouting_t *ctx, FilterMode_t chA, FilterMode_t chB, FilterMode_t chC) {
    if (!ctx || !ctx->initialized || !bcm2835_init()) return;

    // LOW_PASS = 1 (HIGH) -> bcm2835_gpio_set()
    // HIGH_PASS = 0 (LOW)  -> bcm2835_gpio_clr()
    bcm2835_gpio_write(GPIO_SWITCH_CHANNEL_A, chA);
    bcm2835_gpio_write(GPIO_SWITCH_CHANNEL_B, chB);
    bcm2835_gpio_write(GPIO_SWITCH_CHANNEL_C, chC);
}

/**
 * @brief Releases GPIO hardware resources.
 */
void Audio_Cleanup(AudioRouting_t *ctx) {
    if (!ctx || !ctx->initialized) return;

    // Ustaw piny jako INPUT (bezpieczne wyłączenie)
    bcm2835_gpio_fsel(GPIO_SWITCH_CHANNEL_A, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(GPIO_SWITCH_CHANNEL_B, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(GPIO_SWITCH_CHANNEL_C, BCM2835_GPIO_FSEL_INPT);
    
    bcm2835_close();  // Zamknij BCM2835
    
    if (ctx) ctx->initialized = false;
}
