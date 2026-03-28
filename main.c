/* Entry point for the driver */
#include "libraries.h"
#include "pins.h"
#include "shm_writer.h"
#include "adc.h"
#include "potentiometer.h"
#include "switch_logic.h"
#include "wav_writer.h"

static float clampf(float v, float lo, float hi)
{
	if (v < lo) return lo;
	if (v > hi) return hi;
	return v;
}

static int16_t voltage_to_pcm16(float v)
{
	/* MCP3564 path currently yields 0..3.3V-like values, convert to signed PCM around mid-scale. */
	const float normalized = clampf((v / 3.3f) * 2.0f - 1.0f, -1.0f, 1.0f);
	if (normalized >= 0.0f) {
		return (int16_t)(normalized * 32767.0f);
	}
	return (int16_t)(normalized * 32768.0f);
}

int main(void){
	int exit_code = EXIT_FAILURE;
	SharedBuffer *my_buffer = NULL; // Pointer to shared memory buffer
	AudioRouting_t ctx = {.initialized = false};
	WavWriter wav = {0};
	bool wav_enabled = false;
	uint32_t wav_frames_target = 0;
	uint32_t wav_frames_written = 0;
	uint16_t wav_channels = 4;

	const char *wav_path = getenv("WAV_OUTPUT_PATH");
	const char *wav_seconds_env = getenv("WAV_SECONDS");
	const char *wav_channels_env = getenv("WAV_CHANNELS");
	uint32_t wav_seconds = 10U;
	if (wav_seconds_env && wav_seconds_env[0] != '\0') {
		const long parsed = strtol(wav_seconds_env, NULL, 10);
		if (parsed > 0) wav_seconds = (uint32_t)parsed;
	}
	if (wav_channels_env && wav_channels_env[0] != '\0') {
		const long parsed = strtol(wav_channels_env, NULL, 10);
		if (parsed >= 1 && parsed <= 4) wav_channels = (uint16_t)parsed;
	}

	if (!hardware_init()) {
        fprintf(stderr, "Failed to initialize hardware layer!\n");
        goto cleanup;
    }
	fprintf(stdout, "Hardware layer initialized\n");

	if (!Audio_Init(&ctx)){ // Initialize audio context
		fprintf(stderr, "Failed to initialize audio routing!\n");
		goto cleanup;
	}
	fprintf(stdout, "Audio routing initialized\n");

	Audio_SetFilters(&ctx, FILTER_LOW_PASS, FILTER_LOW_PASS, FILTER_LOW_PASS); // Default to low-pass filters
	
	mcp4011_init_all(); // Initialize all potentiometers
	mcp4011_set_all(0);  // Start with all pots at position 0
	fprintf(stdout, "Potentiometers initialized\n");

	if (shm_initialize(SHM_NAME, &my_buffer) != 0) { // Initialize shared memory for inter-process communication
        fprintf(stderr, "Error initializing SHM\n");
        goto cleanup;
    }
	fprintf(stdout, "Shared memory initialized: %s\n", SHM_NAME);

	if (!init_raspberry_pi_spi(SPI_CS, BCM2835_SPI_CLOCK_DIVIDER_16)) { // Initialize SPI for ADC communication
		fprintf(stderr, "Failed to initialize SPI\n");
        goto cleanup;
    }
	fprintf(stdout, "SPI initialization complete\n");

    init_mcp3564(); // Initialize the ADC
	fprintf(stdout, "MCP3564 initialized\n");

    float channel_values[8]; // Buffer to hold the ADC values for all channels
	int16_t wav_frame_i16[4];

	if (wav_path && wav_path[0] != '\0') {
		if (!wav_writer_open(&wav, wav_path, wav_channels, WRITE_FREQ, 16U)) {
			fprintf(stderr, "Failed to initialize WAV output: %s\n", wav_path);
			goto cleanup;
		}
		wav_enabled = true;
		wav_frames_target = wav_seconds * WRITE_FREQ;
		fprintf(stdout, "WAV recording enabled: %s (%u ch, %u Hz, %u s)\n",
				wav_path, wav_channels, WRITE_FREQ, wav_seconds);
	}

	uint32_t loop_counter = 0;
	uint32_t next_log_frame = WRITE_FREQ;

	
	while(true) {
        printf("Reading all channels\n");
    	read_all_channels(channel_values); // Read initial values to ensure everything is working
		++loop_counter;
		if(shm_write(my_buffer, channel_values) != 0){ // Write ADC values to shared memory for other processes to read
			fprintf(stderr, "Error writing to SHM\n");
			break; 
		}

		if (wav_enabled && wav_frames_written < wav_frames_target) {
			for (uint16_t ch = 0; ch < wav_channels; ++ch) {
                printf("Voltage to PCM16\n");
				wav_frame_i16[ch] = voltage_to_pcm16(channel_values[ch]);
                fprintf(stdout, "Data written: %d\n", wav_frame_i16[ch]);
			}
			if (!wav_writer_write_interleaved_i16(&wav, wav_frame_i16, 1U)) {
				fprintf(stderr, "WAV write failed\n");
				break;
			}
			++wav_frames_written;
			if (wav_frames_written >= wav_frames_target) {
				fprintf(stdout, "WAV recording complete: %u frames\n", wav_frames_written);
				break;
			}
		}

		if (loop_counter >= next_log_frame) {
			fprintf(stdout,
				"Capture progress: frames=%u ch0=%.5fV ch1=%.5fV ch2=%.5fV ch3=%.5fV",
				loop_counter,
				channel_values[0], channel_values[1],
				channel_values[2], channel_values[3]);
			if (wav_enabled) {
				fprintf(stdout, " wav=%u/%u", wav_frames_written, wav_frames_target);
			}
			fprintf(stdout, "\n");
			next_log_frame += WRITE_FREQ;
		}
	}

	exit_code = EXIT_SUCCESS;

cleanup:
	if (my_buffer) {
		shm_cleanup(SHM_NAME, my_buffer); // Clean up shared memory resources
	}
	Audio_Cleanup(&ctx); // Clean up GPIO resources before exiting
	cleanup_raspberry_pi_spi(); // Clean up SPI resources
	if (wav_enabled && !wav_writer_close(&wav)) {
		fprintf(stderr, "Failed to finalize WAV file header\n");
	}
	hardware_cleanup(); // Release GPIO and SPI state

	return exit_code;
}
