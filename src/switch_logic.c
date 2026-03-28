#include "switch_logic.h"

/**
 * @brief Initializes the GPIO lines used for filter routing.
 * @return true on success.
 */
bool Audio_Init(AudioRouting_t *ctx) {
    if (!gpio_request_output(GPIO_SWITCH_CHANNEL_A, HIGH) ||
        !gpio_request_output(GPIO_SWITCH_CHANNEL_B, LOW) ||
        !gpio_request_output(GPIO_SWITCH_CHANNEL_C, LOW)) {
        return false;
    }
    
    if (ctx) ctx->initialized = true;
    return true;
}

/**
 * @brief Sets the signal path for all channels atomically.
 */
void Audio_SetFilters(AudioRouting_t *ctx, FilterMode_t chA, FilterMode_t chB, FilterMode_t chC) {
    if (!ctx || !ctx->initialized) return;

    gpio_write_output(GPIO_SWITCH_CHANNEL_A, chA);
    gpio_write_output(GPIO_SWITCH_CHANNEL_B, chB);
    gpio_write_output(GPIO_SWITCH_CHANNEL_C, chC);
}

/**
 * @brief Releases GPIO hardware resources.
 */
void Audio_Cleanup(AudioRouting_t *ctx) {
    if (!ctx || !ctx->initialized) return;

    gpio_release(GPIO_SWITCH_CHANNEL_A);
    gpio_release(GPIO_SWITCH_CHANNEL_B);
    gpio_release(GPIO_SWITCH_CHANNEL_C);
    
    if (ctx) ctx->initialized = false;
}
