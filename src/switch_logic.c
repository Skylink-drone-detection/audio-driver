#include <gpiod.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "pins.h"


/**
 * @brief Filter Selection Logic.
 * Logic 0 (LOW)  -> Routes to Y0 (High-Pass Filter) [cite: 197, 202]
 * Logic 1 (HIGH) -> Routes to Y1 (Low-Pass Filter) [cite: 178, 183]
 */
typedef enum {
    FILTER_LOW_PASS  = 1,
    FILTER_HIGH_PASS = 0
} FilterMode_t;

/* Context structure to maintain the GPIO connection handle */
typedef struct {
    struct gpiod_chip *chip;
    struct gpiod_line_request *request;
} AudioRouting_t;


/**
 * @brief Initializes the GPIO chip and lines.
 * @return true on success.
 */
bool Audio_Init(AudioRouting_t *ctx) {
    /* Raspberry Pi 5 uses gpiochip4 for the 40-pin header */
    ctx->chip = gpiod_chip_open_by_name("/dev/gpiochip4");
    if (!ctx->chip) return false;

    unsigned int pins[] = {GPIO_SWITCH_CHANNEL_A , GPIO_SWITCH_CHANNEL_B , GPIO_SWITCH_CHANNEL_C };

    /* Request 3 lines as outputs initialized to LOW (Low-Pass) [cite: 197] */
    ctx->request = gpiod_chip_request_lines(ctx->chip, NULL, 
        gpiod_line_config_new_ext(pins, 3, 
        gpiod_line_settings_new_ext(GPIOD_LINE_DIRECTION_OUTPUT, 0)));

    return (ctx->request != NULL);
}


/**
 * @brief Sets the signal path for all channels atomically.
 */
void Audio_SetFilters(AudioRouting_t *ctx, FilterMode_t chA, FilterMode_t chB, FilterMode_t chC) {
    if (!ctx || !ctx->request) return;

    enum gpiod_line_value values[] = {
        (chA) ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE,
        (chB) ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE,
        (chC) ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE
    };

    /* Update hardware switches simultaneously  */
    gpiod_line_request_set_values(ctx->request, values);
}


/**
 * @brief Releases GPIO hardware resources.
 */
void Audio_Cleanup(AudioRouting_t *ctx) {
    if (ctx->request) gpiod_line_request_release(ctx->request);
    if (ctx->chip) gpiod_chip_close(ctx->chip);
}

