#include <stddef.h>

extern char end;
static char* heap = &end;

void* kmalloc(size_t size) {
	void* ptr = heap;
	heap += size;
	return ptr;
}
