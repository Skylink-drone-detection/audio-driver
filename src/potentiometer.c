#include "potentiometer.h"

// GPIO helpers - ZMIENIONE na bcm2835
void gpio_set(uint8_t gpio, uint8_t value){
    bcm2835_gpio_fsel(gpio, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(gpio, value);
}

// Inicjalizacja pojedynczego MCP4011
void mcp4011_init_single(mcp4011_t *pot, uint8_t cs, uint8_t ud, float r_ab_value){
    pot->cs_pin = cs;
    pot->ud_pin = ud;
    pot->r_ab = r_ab_value;
    pot->current_wiper_pos = POR_WIPER_MID;  // POR default
    
    gpio_set(cs, HIGH);  // CS idle HIGH
    gpio_set(ud, HIGH);  // U/D default UP
}

// Inicjalizacja wszystkich 8 potencjometrów
void mcp4011_init_all(void){
    if (!bcm2835_init()) {
        fprintf(stderr, "bcm2835_init failed. Run with sudo\n");
        exit(EXIT_FAILURE);
    }
    
    // Wspólne piny
    gpio_set(CS_GPIO, HIGH);
    gpio_set(INC_GPIO, HIGH);
    
    // Inicjalizuj 8 potencjometrów (10kΩ)
    for (uint8_t i = 0; i < NUM_POTS; ++i){
        mcp4011_init_single(&pots[i], CS_GPIO, UD_BASE + i, R_AB_3);
    }
}

void mcp4011_set_position(uint8_t pot_num, uint8_t position){
    if (pot_num >= NUM_POTS || position > 63) {
        return;
    }
    
    mcp4011_t *pot = &pots[pot_num];
    uint8_t current = pot->current_wiper_pos;
    
    if (current == position) return; 
    
    uint8_t steps = (position > current) ? (position - current) : (current - position);
    
    // 1. Kierunek U/D
    gpio_set(pot->ud_pin, position > current);
    
    // 2. CS LOW (aktywacja)
    gpio_set(pot->cs_pin, LOW);
    
    for(uint8_t i = 0; i < steps; ++i){
        gpio_set(INC_GPIO, LOW);     // ↓ 10μs
        bcm2835_delayMicroseconds(10);
        gpio_set(INC_GPIO, HIGH);    // ↑
        bcm2835_delayMicroseconds(10);
    }
    
    // 4. CS HIGH (zapis)
    gpio_set(pot->cs_pin, HIGH);
    
    pot->current_wiper_pos = position;
}

// Ustaw wszystkie na tę samą pozycję
void mcp4011_set_all(uint8_t position){
    for(int i = 0; i < NUM_POTS; ++i){
        mcp4011_set_position(i, position);
    }
}

// Obliczanie oporu wipera
float mcp4011_get_r_wiper(uint8_t pot_num){
    return WIPER_RESISTANCE(pots[pot_num].r_ab, pots[pot_num].current_wiper_pos);
}
