#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#ifndef __always_inline
#define __always_inline inline __attribute__((always_inline))
#endif

/* ------ ALLOCATOR PARAMETERS ------ */

#define MIN_CAPACITY 32768
#define MAX_CAPACITY 67108864
#define ALIGNMENT 16
#define CHUNK_HEADER_SZ 16
#define MIN_CHUNK_SZ 32
#define SIZE_SZ 8

#define PREV_IN_USE 0x1
#define SBRK_FAILURE ((void *)(-1))

/* ------ UTILS ------ */ 

__always_inline uint64_t max_unsigned(uint64_t a, uint64_t b)
{
	return a > b ? a : b;
}

__always_inline size_t align(size_t bs)
{
	return (bs + (ALIGNMENT-1)) & ~(ALIGNMENT-1);
}

/* ------ STRUCTS ------ */

struct tpin_chunk {
	size_t prev_size;
	size_t size;
	/*
	 * Flags for size
	 * 0x8 : --
	 * 0x4 : --
	 * 0x2 : --
	 * 0x1 : prev_in_use
	 */
	struct tpin_chunk *next;
	struct tpin_chunk *prev;
};

struct tpin_heap {
	size_t size;
	struct tpin_chunk *top;
	struct tpin_chunk *free;
};

__always_inline struct tpin_chunk *chunk_at_offset(struct tpin_chunk *c, size_t offset)
{
	return (struct tpin_chunk *) ((uint8_t *)c + offset);
}

__always_inline void set_chunk_in_use(struct tpin_chunk *c)
{
	struct tpin_chunk *target = chunk_at_offset(c, c->size);
	target->size = target->size | PREV_IN_USE;
}

__always_inline void *chunk_to_mem(struct tpin_chunk *c)
{
	return (void *) ((uint8_t *)c + SIZE_SZ*2);
}

__always_inline struct tpin_chunk *user_to_chunk(void *mem)
{
	return (struct tpin_chunk *) ((uint8_t *)mem - SIZE_SZ*2);
}

__always_inline size_t get_chunk_size(struct tpin_chunk *c)
{
	return (c->size & ~0x15);
}

uintptr_t current_brk = 0;

struct tpin_heap _heap= { 0 };
struct tpin_heap *heap= &_heap;

/* 
 * Add padding to requested bytes to account for chunk metadata
 * only one SIZE_SZ is added to the padding because prev_size belongs
 * to previous chunk when prev_in_use bit is set to 1.
 */
__always_inline size_t request_to_size(size_t bs)
{
	return max_unsigned(MIN_CHUNK_SZ, align(bs + SIZE_SZ));
}

int malloc_init()
{
	void *old_brk = sbrk(MIN_CAPACITY);
	if(old_brk == SBRK_FAILURE) return 1;

	current_brk = (uintptr_t) ((uint8_t *)old_brk + MIN_CAPACITY);

	heap->size = MIN_CAPACITY;

	//Set initial top
	struct tpin_chunk *top = (struct tpin_chunk *)old_brk;

	top->size = MIN_CAPACITY;
	top->next = top->prev = NULL;

	heap->top  = top;
	heap->free = NULL;
}

struct tpin_chunk *chunk_from_top(size_t sz)
{
	struct tpin_chunk *old_top = heap->top;
	struct tpin_chunk *new_top;
	struct tpin_chunk *new;

	size_t old_size = old_top->size;

	new = old_top;
	new->size = sz;
	new->size |= (old_size & PREV_IN_USE);

	new_top = (struct tpin_chunk *)((uint8_t *)heap->top + sz);
	new_top->size = old_size - sz;
	new_top->next = NULL;
	new_top->prev = new;

	heap->top = new_top;

	return new;
}

void *tpin_malloc(size_t req)
{
	size_t sz = request_to_size(req);

	assert(sz < MAX_CAPACITY && sz + heap->size < MAX_CAPACITY);
	if (sz > MAX_CAPACITY || sz + heap->size > MAX_CAPACITY) return NULL;

	if (heap->size == 0) {
		malloc_init();
	}

	printf("Requested bytes: %zu\n", sz);
	printf("Returned chunk size: %zu\n", sz);
	printf("Top address: %p\n", heap->top);

	printf("Looking for suitable free chunk...\n");

	//check for a suitable free chunk
	if (heap->free) {
		return NULL;
	}

	printf("No suitable free chunk found, attempting to split top...\n");

	if (sz + MIN_CHUNK_SZ > heap->top->size) {
		// TODO: IMPLEMENT EXPAND HEAP
		printf("Top size not enough for split");
		return NULL;
	}

	// do top split
	struct tpin_chunk *chunk = chunk_from_top(sz);
	if (!chunk) return NULL;
	
	printf("Returned chunk address: %p\n", chunk);
	printf("New top address: %p\n", heap->top);

	set_chunk_in_use(chunk);

	printf("---------------------------\n");

	return chunk_to_mem(chunk);
}

int main()
{
	int *xs = (int *) tpin_malloc(sizeof(int) * 10);
	int *ys = (int *) tpin_malloc(sizeof(int) * 10);
	if (!xs || !ys) {
		printf("Memory not Allocated\n");
		return 0;
	}
	for (int i = 0; i < 10; i++) {
		xs[i] = i;
		ys[i] = i*2;
	}
	for (int *ptr = xs; ptr < xs + 10; ptr++) {
		printf("Value: %d \t Address: %p\n", *ptr, (void *)ptr);
	}
	printf("-----------\n");
	for (int *ptr = ys; ptr < ys + 10; ptr++) {
		printf("Value: %d \t Address: %p\n", *ptr, (void *)ptr);
	}
	return 0;
}
