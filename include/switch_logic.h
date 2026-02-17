#ifndef SWITCH_H
#define SWITCH_H

#include "libraries.h"
#include "pins.h"

/**
 * @brief Filter Selection Logic.
 * Logic 0 (LOW)  -> Routes to Y0 (High-Pass Filter)
 * Logic 1 (HIGH) -> Routes to Y1 (Low-Pass Filter)
 */
typedef enum {
    FILTER_LOW_PASS  = 1,
    FILTER_HIGH_PASS = 0
} FilterMode_t;

/* Context structure to maintain the BCM2835 handle */
typedef struct {
    bool initialized;
} AudioRouting_t;

/**
 * @brief Initializes the BCM2835 library and GPIO pins.
 * @return true on success.
 */
bool Audio_Init(AudioRouting_t *ctx);
/**
 * @brief Sets the signal path for all channels atomically.
 */
void Audio_SetFilters(AudioRouting_t *ctx, FilterMode_t chA, FilterMode_t chB, FilterMode_t chC);
/**
 * @brief Releases GPIO hardware resources.
 */
void Audio_Cleanup(AudioRouting_t *ctx);

#endif