/* Inter-process communication with Shared Memory
 * through Ring Buffer of microphone data arrays
*/
#ifndef SHM_WRITER_H
#define SHM_WRITER_H
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdatomic.h>
#include <time.h>
#include <string.h>

#define SHM_NAME "/mic_ring_buffer"
#define WRITE_FREQ 8000
#define BUFFER_SIZE 800
#define ARRAY_SIZE 8

// Ring Buffer shared between processes
typedef struct {
	atomic_uint head; // write index
	atomic_uint tail; // read index
	float data[BUFFER_SIZE][ARRAY_SIZE];
} SharedBuffer;

// Initialize shared memory
int shm_initialize(const char *shm_name, SharedBuffer **buffer_out);

// Write one array to buffer
int shm_write(SharedBuffer *buffer, float data[ARRAY_SIZE]);

// Clean up shared memory (unmap and unlink)
int shm_cleanup(const char *shm_name, SharedBuffer *buffer);

#endif // SHM_WRITER_H

