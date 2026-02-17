/* Entry point for the driver */
#include "libraries.h"
#include "pins.h"
#include "shm_writer.h"
#include "adc.h"

int main(void){

	if (!bcm2835_init()) {
        fprintf(stderr, "Cannot initialize BCM2835!\n");
        return EXIT_FAILURE;
    }

	AudioRouting_t ctx = {.initialized = false};
	if (!Audio_Init(&ctx)){ // Initialize audio context
		fprintf(stderr, "Failed to initialize audio routing!\n");
		return EXIT_FAILURE;
	}

	Audio_SetFilters(&ctx, FILTER_LOW_PASS, FILTER_LOW_PASS, FILTER_LOW_PASS); // Default to low-pass filters
	
	mcp4011_init_all(); // Initialize all potentiometers
	mcp4011_set_all(0);  // Start with all pots at position 0

	SharedBuffer *my_buffer = NULL; // Pointer to shared memory buffer

	if (shm_initialize(SHM_NAME, &my_buffer) != 0) { // Initialize shared memory for inter-process communication
        fprintf(stderr, "Error initializing SHM\n");
        return EXIT_FAILURE;
    }

	if (!init_raspberry_pi_spi(SPI_CS, BCM2835_SPI_CLOCK_DIVIDER_16)) { // Initialize SPI for ADC communication
		fprintf(stderr, "Failed to initialize SPI\n");
        return EXIT_FAILURE;
    }

    init_mcp3564(); // Initialize the ADC

    float channel_values[8]; // Buffer to hold the ADC values for all channels

	
	while(true) {
    	read_all_channels(channel_values); // Read initial values to ensure everything is working
		if(shm_write(my_buffer, channel_values) != 0){ // Write ADC values to shared memory for other processes to read
			fprintf(stderr, "Error writing to SHM\n");
			break; 
		}
	}

	//CLEAN UP
	shm_cleanup(SHM_NAME, my_buffer); // Clean up shared memory resources
	Audio_Cleanup(&ctx); // Clean up GPIO resources before exiting
	cleanup_raspberry_pi_spi(); // Clean up SPI resources


	return EXIT_SUCCESS;
}

