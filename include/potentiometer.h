#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H 1

#include "libraries.h"
#include "pins.h"

/***********************************************************************************
                PIN FUNCTION TABLE
MCP4011 (SOIC-8)    Symbol  Pin Type    Buffer Type     Function
      1             V_DD     P            -           Positive Power Supply Input
      2             V_SS     P            -           Ground
      3             A         I/O          A           Potentiometer Terminal A
      4             W         I/O          A           Potentiometer Wiper Terminal
      5             CS        I            TTL         Chip Select Input
      6             B         I/O          A           Potentiometer Terminal B
      7             NC        -            -           No Connection
      8             U/D       I            TTL         Increment/Decrement Input

Legend: TTL = TTL compatible input
        I   = Input
        P   = Power
        A   = Analog input
        O   = Ouput
************************************************************************************/

constexpr uint8_t NUMBER_OF_PINS = 8;

constexpr uint8_t PIN_NUMBER_1   = 1;
constexpr uint8_t PIN_NUMBER_2   = 2;
constexpr uint8_t PIN_NUMBER_3   = 3;
constexpr uint8_t PIN_NUMBER_4   = 4;
constexpr uint8_t PIN_NUMBER_5   = 5;
constexpr uint8_t PIN_NUMBER_6   = 6;
constexpr uint8_t PIN_NUMBER_7   = 7;
constexpr uint8_t PIN_NUMBER_8   = 8;

enum pins : uint8_t {
    V_DD = PIN_NUMBER_1,
    V_SS = PIN_NUMBER_2,
    A    = PIN_NUMBER_3,
    W    = PIN_NUMBER_4,
    CS   = PIN_NUMBER_5,
    B    = PIN_NUMBER_6,
    NC   = PIN_NUMBER_7,
    UD   = PIN_NUMBER_8  
};

// V_DD is relative to V_SS and can range from 1.8V to 5.5v
constexpr float MINIMUM_V_DD = 1.8f; 
constexpr float MAXIMUM_V_DD = 5.5f;

// Nominal resistances R_AB
constexpr float R_AB_1 = 2.1f;   // 2.1 kΩ
constexpr float R_AB_2 = 5.0f;   // 5.0 kΩ
constexpr float R_AB_3 = 10.0f;  // 10 kΩ  
constexpr float R_AB_4 = 50.0f;  // 50 kΩ

// 63 resistors = 64 positions (0-63)
constexpr uint8_t NUMBER_OF_RESISTORS = 63;
constexpr uint8_t NUMBER_OF_POSSIBLE_SETTINGS = NUMBER_OF_RESISTORS + 1;

// Step resistance: R_S = R_AB / 63
#define STEP_RESISTANCE(r_ab) ((r_ab) / 63.0f)

// Resistance between wiper and terminal B: R_WB = ((R_AB * N) / 63) + R_W
#define WIPER_RESISTANCE(r_ab, n) ((r_ab * (n)) / 63.0f)

// Signal tolerance up to 12.5V
constexpr float MAXIMUM_TOLERANCE_SIGNAL = 12.5f;

// COMPLETED TODOs z datasheet
constexpr float CS_V_IH = 2.0f;   // Logic HIGH min
constexpr float CS_V_IL = 0.8f;   // Logic LOW max
constexpr float V_TP    = 1.5f;   // BOR trip point

constexpr uint8_t POR_WIPER_MID = 0x1F // 31/63 = 50% mid-scale (z tabeli)

/****************************************************************************/

typedef struct {
    uint8_t cs_pin;
    uint8_t ud_pin;
    uint8_t current_wiper_pos;  // 0-63
    float r_ab;                 // value R_AB
} mcp4011_t;


// GPIO helpers - ZMIENIONE na bcm2835
void gpio_set(uint8_t gpio, uint8_t value);

// Inicjalizacja pojedynczego MCP4011
void mcp4011_init_single(mcp4011_t *pot, uint8_t cs, uint8_t ud, float r_ab_value);

// Inicjalizacja wszystkich 8 potencjometrów
void mcp4011_init_all(void);

void mcp4011_set_position(uint8_t pot_num, uint8_t position);

// Ustaw wszystkie na tę samą pozycję
void mcp4011_set_all(uint8_t position);

// Obliczanie oporu wipera
float mcp4011_get_r_wiper(uint8_t pot_num);

#endif