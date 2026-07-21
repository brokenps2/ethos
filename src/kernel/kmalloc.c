#include <stddef.h>

extern char _kernel_end;
static char* heap = &_kernel_end;

void* kmalloc(size_t size) {
	void* ptr = heap;
	heap += size;
	return ptr;
}
