#include <stdlib.h>

#include "arena.h"

MemoryBlock *
CreateArena()
{
	MemoryBlock *arena = malloc(sizeof(MemoryBlock));
	arena->next        = NULL;
	arena->free        = 0;
	return arena;
}

void
DestroyArena(MemoryBlock *arena)
{
	MemoryBlock *tmp;
	do {
		tmp = arena->next;
		free(arena);
		arena = tmp;
	} while (tmp);
}

void *
ArenaAlloc(MemoryBlock *arena, int len)
{
	void *address;
	if (len % 8) len += 8 - len % 8;
	while (sizeof(arena->block) - arena->free < len) {
		if (arena->next) arena = arena->next;
		else {
			arena->next       = malloc(sizeof(MemoryBlock));
			arena->next->next = NULL;
			arena->next->free = 0;
			arena             = arena->next;
		}
	}
	address = arena->block + arena->free;
	arena->free += len;
	return address;
}
