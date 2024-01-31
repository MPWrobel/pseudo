#ifndef arena_h
#define arena_h

typedef struct MemoryBlock {
	int                 free;
	struct MemoryBlock *next;
	char                block[1024];
} MemoryBlock;

MemoryBlock *CreateArena();
void         DestroyArena(MemoryBlock *);
void        *ArenaAlloc(MemoryBlock *, int);

#endif /* !arena_h */
