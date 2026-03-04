#include "shm_writer.h"


#define TEST_SHM_NAME "/shm_writer_test"
constexpr uint8_t TEST_ITERATIONS = 128 // 2*BUFFER_SIZE

int main(void) {
	SharedBuffer *buffer = nullptr;

	(void) printf("Initializing shared memory buffer.\n");
	if(shm_initialize(TEST_SHM_NAME, &buffer) != 0){
		fprintf(stderr, "Failed to initialize shared memory buffer\n");
		return 1;
	}

	for(register int i = 0; i < TEST_ITERATIONS; i++){
		float data[ARRAY_SIZE];
		for(register int j = 0; j < ARRAY_SIZE; j++)
			data[j] = (float)(i * 10 + j);

		if (shm_write(buffer, data) != 0){
			fprintf(stderr, "Write failed at iteration %d\n", i);
			shm_cleanup(TEST_SHM_NAME, buffer);
			return 1;
		}

		(void) printf("Wrote iteration %d: ", i);
		for(register int j = 0; j < ARRAY_SIZE; j++)
			(void) printf("%.1f ", data[j]);

		(void) printf("\n");
	}

	(void) printf("Testing data integrity (manual check).\n");
	uint32_t head = atomic_load(&buffer->head);
	uint32_t tail = atomic_load(&buffer->tail);

	(void) printf("Head = %u, Tail = %u\n", head, tail);

	(void) printf("Cleaning up shared memory.\n");
	shm_cleanup(TEST_SHM_NAME, buffer);
	(void) printf("Test complete.\n");

	return EXIT_SUCCESS;
}

