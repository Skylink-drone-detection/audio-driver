
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <wiringPi.h>

/***********************************************************************************
                PIN FUNCTION TABLE
MCP4011 (SOIC-8)    Symbol  Pin Type    Buffer Type     Function
       1             V_DD      P            -           Positive Power Supply Input
       2             V_SS      P            -           Ground
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

#define NUMBER_OF_PINS ((uint8_t) 8)

#define PIN_NUMBER_1   ((uint8_t) 1)
#define PIN_NUMBER_2   ((uint8_t) 2)
#define PIN_NUMBER_3   ((uint8_t) 3)
#define PIN_NUMBER_4   ((uint8_t) 4)
#define PIN_NUMBER_5   ((uint8_t) 5)
#define PIN_NUMBER_6   ((uint8_t) 6)
#define PIN_NUMBER_7   ((uint8_t) 7)
#define PIN_NUMBER_8   ((uint8_t) 8)

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
#define MINIMUM_V_DD   ((float) 1.8f) 
#define MAXIMUM_V_DD   ((float) 5.5f)

// Nominal resistances R_AB
#define R_AB_1        ((float) 2.1f)   // 2.1 kΩ
#define R_AB_2        ((float) 5.0f)   // 5.0 kΩ
#define R_AB_3        ((float) 10.0f)  // 10 kΩ  
#define R_AB_4        ((float) 50.0f)  // 50 kΩ

// 63 resistors = 64 positions (0-63)
#define NUMBER_OF_RESISTORS         ((uint8_t) 63)
#define NUMBER_OF_POSSIBLE_SETTINGS ((uint8_t) NUMBER_OF_RESISTORS + 1)

// Step resistance: R_S = R_AB / 63
#define STEP_RESISTANCE(r_ab) ((r_ab) / 63.0f)

// Resistance between wiper and terminal B: R_WB = ((R_AB * N) / 63) + R_W
#define WIPER_RESISTANCE(r_ab, n) ((r_ab * (n)) / 63.0f)

// Signal tolerance up to 12.5V
#define MAXIMUM_TOLERANCE_SIGNAL ((float) 12.5f)

// COMPLETED TODOs z datasheet
#define CS_V_IH   ((float) 2.0f)   // Logic HIGH min
#define CS_V_IL   ((float) 0.8f)   // Logic LOW max
#define V_TP      ((float) 1.5f)   // BOR trip point

#define POR_WIPER_MID ((uint8_t) 0x1F) // 31/63 = 50% mid-scale (z tabeli)

/****************************************************************************/

#define NUM_POTS 8
#define CS_GPIO  4      // GPIO4 = CS (wspólny)
#define INC_GPIO 17     // GPIO17 = INC (wspólny)
#define UD_BASE  24     // GPIO24-31 = U/D 0-7

typedef struct {
    uint8_t cs_pin;
    uint8_t ud_pin;
    uint8_t current_wiper_pos;  // 0-63
    float r_ab;                 // wartość R_AB
} mcp4011_t;

// Stan 8 potencjometrów
static mcp4011_t pots[NUM_POTS];

// GPIO helpers
void gpio_set(uint8_t gpio, uint8_t value){
    pinMode(gpio, OUTPUT);
    digitalWrite(gpio, value);
}

// Inicjalizacja pojedynczego MCP4011
void mcp4011_init_single(mcp4011_t *pot, uint8_t cs, uint8_t ud, float r_ab_value) {
    pot -> cs_pin = cs;
    pot -> ud_pin = ud;
    pot -> r_ab = r_ab_value;
    pot -> current_wiper_pos = POR_WIPER_MID;  // POR default
    
    gpio_set(cs, 1);  // CS idle HIGH
    gpio_set(ud, 1);  // U/D default UP
}

// Inicjalizacja wszystkich 8 potencjometrów
void mcp4011_init_all(void) {
    wiringPiSetupGpio();
    
    // Wspólne piny
    gpio_set(CS_GPIO, 1);
    gpio_set(INC_GPIO, 1);
    
    // Inicjalizuj 8 potencjometrów (10kΩ)
    for (uint8_t i = 0; i < NUM_POTS; ++i){
        mcp4011_init_single(&pots[i], CS_GPIO, UD_BASE + i, R_AB_3);
    }
}

// TWOJA MATEMATYKA - główna funkcja sterowania!
void mcp4011_set_position(uint8_t pot_num, uint8_t position){
    if (pot_num >= NUM_POTS || position > 63) {
        return;
    }
    
    mcp4011_t *pot = &pots[pot_num];
    uint8_t current = pot -> current_wiper_pos;
    
    if (current == position) return;  // Już OK
    
    // Oblicz kroki
    uint8_t steps = (position > current) ? (position - current) : (current - position);
    
    // 1. Kierunek U/D
    gpio_set(pot -> ud_pin, position > current);
    
    // 2. CS LOW (aktywacja)
    gpio_set(pot -> cs_pin, 0);
    
    // 3. INC pulses (datasheet timing)
    for (uint8_t i = 0; i < steps; ++i){
        gpio_set(INC_GPIO, 0);   // ↓ 10μs
        usleep(10);
        gpio_set(INC_GPIO, 1);   // ↑
        usleep(10);
    }
    
    // 4. CS HIGH (zapis!)
    gpio_set(pot -> cs_pin, 1);
    
    pot -> current_wiper_pos = position;
}

// Ustaw wszystkie na tę samą pozycję
void mcp4011_set_all(uint8_t position){
    for (int i = 0; i < NUM_POTS; i++){
        mcp4011_set_position(i, position);
    }
}

// TWOJA MATEMATYKA - oblicz opór wipera
float mcp4011_get_r_wiper(uint8_t pot_num){
    return WIPER_RESISTANCE(pots[pot_num].r_ab, pots[pot_num].current_wiper_pos);
}

/***********************************************************************************/
// TEST + DEMO (bez printf)
/***********************************************************************************/

int main() {
    // Inicjalizacja
    mcp4011_init_all();
    
    // Test 1: Wszystkie na MAX
    mcp4011_set_all(63);
    sleep(2);
    
    // Test 2: Indywidualne pozycje
    mcp4011_set_position(0, 0);   // Pot 0 = 0%
    mcp4011_set_position(2, 63);  // Pot 2 = 100%
    mcp4011_set_position(4, 32);  // Pot 4 = 50%
    sleep(3);
    
    // Demo: Sweep 0→63→0
    while (1){
        // ↑ 0→63
        for(uint8_t pos = 0; pos <= 63; pos += 4){
            mcp4011_set_all(pos);
            usleep(200000);  // 200ms/step
        }
        
        // ↓ 63→0
        for (uint8_t pos = 63; pos > 0; pos -= 4){
            mcp4011_set_all(pos);
            usleep(200000);
        }
    }
    
    return EXIT_SUCCESS;
}
