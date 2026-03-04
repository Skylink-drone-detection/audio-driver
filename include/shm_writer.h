/* Inter-process communication with Shared Memory
 * through Ring Buffer of microphone data arrays
*/

#ifndef SHM_WRITER_H
#define SHM_WRITER_H 1

#include "libraries.h"
#include "pins.h"

#define SHM_NAME ("/mic_ring_buffer")
constexpr uint16_t WRITE_FREQ = 8000;
constexpr uint16_t BUFFER_SIZE = 800;
constexpr uint8_t ARRAY_SIZE = 8;

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

#endif

