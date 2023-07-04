#ifndef _MALLOC_H_
#define _MALLOC_H_

#define MINSIZE 256
#define BLOCK_MIN_SIZE 16384
#define BLOCK_MEDIUM_SIZE 1048576
#define BLOCK_LARGE_SIZE 33554432
#define MAX_BLOCKS 1000

struct malloc_stats {
	int mallocs;
	int frees;
	int requested_memory;
	int amount_small_blocks;
	int amount_medium_blocks;
	int amount_large_blocks;
};


void *malloc(size_t size);

void free(void *ptr);

void *calloc(size_t nmemb, size_t size);

void *realloc(void *ptr, size_t size);

void get_stats(struct malloc_stats *stats);

#endif  // _MALLOC_H_
