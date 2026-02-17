#include <wiringPi.h>
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
    bool initialized;
} AudioRouting_t;


/**
 * @brief Initializes the GPIO chip and lines.
 * @return true on success.
 */
bool Audio_Init(AudioRouting_t *ctx) {
    if (wiringPiSetupGpio() == -1) {
        fprintf(stderr, "Cannot initialize WiringPi!\n");
        return false;
    }

    int pins[] = {GPIO_SWITCH_CHANNEL_A, GPIO_SWITCH_CHANNEL_B, GPIO_SWITCH_CHANNEL_C};


    for (int i = 0; i < 3; i++) {
        pinMode(pins[i], OUTPUT);
        digitalWrite(pins[i], LOW); 
    }
    if (ctx) ctx->initialized = true;
    return true;
}


/**
 * @brief Sets the signal path for all channels atomically.
 */
void Audio_SetFilters(AudioRouting_t *ctx, FilterMode_t chA, FilterMode_t chB, FilterMode_t chC) {
    if (!ctx || !ctx->initialized) return;

    digitalWrite(GPIO_SWITCH_CHANNEL_A, (chA == FILTER_LOW_PASS) ? HIGH : LOW);
    digitalWrite(GPIO_SWITCH_CHANNEL_B, (chB == FILTER_LOW_PASS) ? HIGH : LOW);
    digitalWrite(GPIO_SWITCH_CHANNEL_C, (chC == FILTER_LOW_PASS) ? HIGH : LOW);
}


/**
 * @brief Releases GPIO hardware resources.
 */
void Audio_Cleanup(AudioRouting_t *ctx) {
    int pins[] = {GPIO_SWITCH_CHANNEL_A, GPIO_SWITCH_CHANNEL_B, GPIO_SWITCH_CHANNEL_C};
    for (int i = 0; i < 3; i++) {
        pinMode(pins[i], INPUT);
    }
    
    if (ctx) ctx->initialized = false;
}

