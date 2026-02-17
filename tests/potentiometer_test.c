#include "../src/mcp4011.c"

int main(void){
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
    while(true){
        // ↑ 0→63
        for(uint8_t pos = 0; pos <= 63; pos += 4){
            mcp4011_set_all(pos);
            usleep(200000);  // 200ms/step
        }
        
        // ↓ 63→0
        for(uint8_t pos = 63; pos > 0; pos -= 4){
            mcp4011_set_all(pos);
            usleep(200000);
        }
    }
    
    return EXIT_SUCCESS;
}
