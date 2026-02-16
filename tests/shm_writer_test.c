#include "shm_writer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TEST_SHM_NAME "/shm_writer_test"
#define TEST_ITERATIONS 128 // 2*BUFFER_SIZE

int main(void) {
	SharedBuffer *buffer = NULL;

	printf("Initializing shared memory buffer.\n");
	if (shm_initialize(SHM_NAME, &buffer) != 0) {
		fprintf(stderr, "Failed to initialize shared memory buffer\n");
		return 1;
	}

	for (int i = 0; i < TEST_ITERATIONS; i++) {
		float data[ARRAY_SIZE];
		for (int j = 0; j < ARRAY_SIZE; j++) {
			data[j] = (float)(i * 10 + j);
		}

		if (shm_write(buffer, data) != 0) {
			fprintf(stderr, "Write failed at iteration %d\n", i);
			shm_cleanup(TEST_SHM_NAME, buffer);
			return 1;
		}

		printf("Wrote iteration %d: ", i);
		for (int j = 0; j < ARRAY_SIZE; j++) {
			printf("%.1f ", data[j]);
		}
		printf("\n");
	}

	printf("Testing data integrity (manual check).\n");
	uint32_t head = atomic_load(&buffer->head);
	uint32_t tail = atomic_load(&buffer->tail);

	printf("Head = %u, Tail = %u\n", head, tail);

	printf("Cleaning up shared memory.\n");
	shm_cleanup(TEST_SHM_NAME, buffer);
	printf("Test complete.\n");

	return 0;
}

