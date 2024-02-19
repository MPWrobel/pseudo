#include <math.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "arena.h"

MemoryBlock *
CreateArena()
{
	MemoryBlock *arena = malloc(sizeof(MemoryBlock));

	arena->block = mmap(NULL, pow(1024, 3), PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);

	// MemoryBlock *arena = malloc(sizeof(MemoryBlock));
	// arena->next = NULL;
	arena->free = 0;
	return arena;
}

void
DestroyArena(MemoryBlock *arena)
{
	// sleep(1);
	munmap(arena->block, pow(1024, 3));
	free(arena);

	// MemoryBlock *tmp;
	// do {
	// 	tmp = arena->next;
	// 	free(arena);
	// 	arena = tmp;
	// } while (tmp);
}

void *
ArenaAlloc(MemoryBlock *arena, int len)
{
	void *address;
	if (len % 8) len += 8 - len % 8;

	// while (sizeof(arena->block) - arena->free < len) {
	// 	if (arena->next) arena = arena->next;
	// 	else {
	// 		arena->next       = malloc(sizeof(MemoryBlock));
	// 		arena->next->next = NULL;
	// 		arena->next->free = 0;
	// 		arena             = arena->next;
	// 	}
	// }

	address = arena->block + arena->free;

	mprotect(address, len, PROT_READ | PROT_WRITE);

	arena->free += len;
	return address;
}
