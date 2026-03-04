#include "shm_writer.h"

int shm_initialize(const char *shm_name, SharedBuffer **buffer_out){
	int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
	if(shm_fd == -1){
		perror("[Shared Memory Writer] shm_open failed");
		return -1;
	}
	if(ftruncate(shm_fd, sizeof(SharedBuffer)) == -1){
		perror("[Shared Memory Writer] ftruncate failed");
		return -1;
	}

	SharedBuffer *buffer = mmap(nullptr, sizeof(SharedBuffer),
								PROT_READ | PROT_WRITE,
								MAP_SHARED, shm_fd, 0);
	if(buffer == MAP_FAILED){
		perror("[Shared Memory Writer] mmap failed");
		return -1;
	}

	atomic_store(&buffer->head, 0);
	atomic_store(&buffer->tail, 0);

	*buffer_out = buffer;

	return 0;
}

int shm_write(SharedBuffer *buffer, float data[ARRAY_SIZE]){
	if(!buffer || !data) return -1;

	uint32_t head = atomic_load_explicit(&buffer->head, memory_order_relaxed);
	uint32_t tail = atomic_load_explicit(&buffer->tail, memory_order_acquire);
	uint32_t next = (head + 1) % BUFFER_SIZE;

	if(next == tail) // drop oldest if buffer is full
		atomic_store_explicit(&buffer->tail, (tail + 1) % BUFFER_SIZE, memory_order_release);

	memcpy(buffer->data[head], data, sizeof(float) * ARRAY_SIZE);

	atomic_thread_fence(memory_order_release);
	atomic_store_explicit(&buffer->head, next, memory_order_release);
	
	return 0;
}

int shm_cleanup(const char *shm_name, SharedBuffer *buffer){
	if(!buffer) return -1;

	munmap(buffer, sizeof(SharedBuffer));
	shm_unlink(shm_name);

	return 0;
}

