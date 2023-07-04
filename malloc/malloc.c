#define _DEFAULT_SOURCE

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include "malloc.h"
#include "printfmt.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>


#define ALIGN4(s) (((((s) -1) >> 2) << 2) + 4)
#define REGION2PTR(r) ((r) + 1)
#define PTR2REGION(ptr) ((struct region *) (ptr) -1)
#define ERROR -1
#define SUCCESS 0

struct region {
	bool free;
	size_t size;
	struct region *next;
};

struct region *small_blocks[MAX_BLOCKS];
struct region *medium_blocks[MAX_BLOCKS];
struct region *large_blocks[MAX_BLOCKS];

int amount_of_mallocs = 0;
int amount_of_frees = 0;
int requested_memory = 0;
int amount_small_blocks = 0;
int amount_medium_blocks = 0;
int amount_large_blocks = 0;

const size_t HEADER_SIZE = sizeof(struct region);

size_t
get_block_size(size_t block_size)
{
	if (block_size + HEADER_SIZE <= BLOCK_MIN_SIZE)
		return BLOCK_MIN_SIZE;
	else if (block_size + HEADER_SIZE <= BLOCK_MEDIUM_SIZE)
		return BLOCK_MEDIUM_SIZE;
	else
		return BLOCK_LARGE_SIZE;
}

static struct region *
split_region(struct region *current_region, size_t size)
{
	struct region *remaining_region =
	        (struct region *) ((char *) current_region + size + HEADER_SIZE);
	struct region *new_region = current_region;

	remaining_region->next = current_region->next;
	remaining_region->size = current_region->size - (size + HEADER_SIZE);
	remaining_region->free = true;

	new_region->size = size;
	new_region->free = false;
	new_region->next = remaining_region;

	return new_region;
}


struct region **
get_current_block(size_t size)
{
	size_t block_size = get_block_size(size);
	if (block_size == BLOCK_MIN_SIZE) {
		return small_blocks;
	} else if (block_size == BLOCK_MEDIUM_SIZE)
		return medium_blocks;
	else if (block_size == BLOCK_LARGE_SIZE)
		return large_blocks;
	else
		return NULL;
}


static struct region *
first_fit(size_t size)
{
	struct region **blocks = get_current_block(size);
	struct region *current_region = NULL;

	for (int i = 0; i < MAX_BLOCKS; i++) {
		current_region = blocks[i];

		while (current_region) {
			if (current_region->size >= size + HEADER_SIZE &&
			    current_region->free) {
				if (current_region->size - size >=
				    HEADER_SIZE +
				            MINSIZE) {  // a la region le sobra más del min
					return split_region(current_region, size);
				} else {
					current_region->free = false;
					return current_region;
				}
			} else {
				current_region = current_region->next;
			}
		}
	}
	// No se encontro ninguna region libre con el tamanio solicitado
	return NULL;
}


static struct region *
best_fit(size_t size)
{
	struct region **blocks = get_current_block(size);
	struct region *current_region = NULL;
	struct region *best_region = NULL;

	for (int i = 0; i < MAX_BLOCKS; i++) {
		current_region = blocks[i];

		while (current_region) {
			if (current_region->size >= size + HEADER_SIZE &&
			    current_region->free) {
				if (best_region == NULL ||
				    current_region->size < best_region->size) {
					best_region = current_region;
				}
			}
			current_region = current_region->next;
		}
	}

	if (best_region != NULL &&
	    best_region->size - size >=
	            HEADER_SIZE + MINSIZE) {  // a la region le sobra más del min
		return split_region(best_region, size);
	} else if (best_region != NULL) {
		best_region->free = false;
		return best_region;
	}

	// No se encontro ninguna region libre con el tamanio solicitado
	return NULL;
}


static struct region *
find_free_region(size_t size)
{
#ifdef FIRST_FIT
	return first_fit(size);
#endif

#ifdef BEST_FIT
	return best_fit(size);
#endif
}

void
add_new_block(struct region **blocks, struct region *new_block)
{
	size_t block_size = get_block_size(new_block->size);
	for (int i = 0; i < MAX_BLOCKS; i++) {
		if (blocks[i] == NULL) {
			blocks[i] = new_block;
			if (block_size == BLOCK_MIN_SIZE) {
				amount_small_blocks++;
			} else if (block_size == BLOCK_MEDIUM_SIZE) {
				amount_medium_blocks++;
			} else if (block_size == BLOCK_LARGE_SIZE) {
				amount_large_blocks++;
			}

			return;
		}
	}
	munmap(new_block, block_size);  // Se llegó al máximo de bloques
	errno = ENOMEM;
}


struct region *
grow_heap(size_t size)
{
	struct region *new_block = NULL;
	size_t block_size = get_block_size(size);
	void *memory = mmap(NULL,
	                    block_size,
	                    PROT_READ | PROT_WRITE,
	                    MAP_ANONYMOUS | MAP_PRIVATE,
	                    -1,
	                    0);

	if (memory == MAP_FAILED) {
		errno = ENOMEM;
		return NULL;
	}

	new_block = memory;
	new_block->size = block_size - HEADER_SIZE;
	new_block->next = NULL;
	new_block->free = true;

	// Agrego el nuevo bloque a la lista de bloques
	add_new_block(get_current_block(size), new_block);
	if (errno == ENOMEM)
		return NULL;

	return new_block;
}

void
merge_regions(struct region *region1, struct region *region2)
{
	region1->size += region2->size;
	region1->next = region2->next;
}

void
coalesce_regions(struct region *block)
{
	struct region *current_region = block;
	struct region *next_region = NULL;

	while (current_region) {
		if (current_region->free) {
			next_region = current_region->next;
			if (next_region && next_region->free) {
				merge_regions(current_region, next_region);
			};
		}
		current_region = current_region->next;
	}
}

bool
all_regions_are_freed(struct region *region)
{
	struct region *current_region = region;
	while (current_region) {
		if (!current_region->free) {
			return false;
		}
		current_region = current_region->next;
	}
	return true;
}

bool
is_small_region_addr(struct region *region, int *block_number)
{
	for (int i = 0; i < MAX_BLOCKS; i++) {
		struct region *region_actual = small_blocks[i];
		*block_number = i;

		while (region_actual) {
			if (region_actual == region) {
				return true;
			}

			region_actual = region_actual->next;
		}
	}
	return false;
}

bool
is_medium_region_addr(struct region *region, int *block_number)
{
	for (int i = 0; i < MAX_BLOCKS; i++) {
		struct region *region_actual = medium_blocks[i];
		*block_number = i;
		while (region_actual) {
			if (region_actual == region)
				return true;

			region_actual = region_actual->next;
		}
	}
	return false;
}

bool
is_large_region_addr(struct region *region, int *block_number)
{
	for (int i = 0; i < MAX_BLOCKS; i++) {
		struct region *region_actual = large_blocks[i];
		*block_number = i;
		while (region_actual) {
			if (region_actual == region)
				return true;

			region_actual = region_actual->next;
		}
	}
	return false;
}

size_t
size_of_block(struct region *region, int *block_number)
{
	if (is_small_region_addr(region, block_number)) {
		return BLOCK_MIN_SIZE;
	} else if (is_medium_region_addr(region, block_number)) {
		return BLOCK_MEDIUM_SIZE;
	} else if (is_large_region_addr(region, block_number)) {
		return BLOCK_LARGE_SIZE;
	} else {
		return 0;
	}
}

struct region *
get_block_addr(size_t size, int number)
{
	switch (size) {
	case BLOCK_MIN_SIZE:
		return small_blocks[number];
	case BLOCK_MEDIUM_SIZE:
		return medium_blocks[number];
	case BLOCK_LARGE_SIZE:
		return large_blocks[number];
	default:
		return NULL;
	}
}

void
point_block_to_null(int block_number, size_t size_block)
{
	switch (size_block) {
	case BLOCK_MIN_SIZE:
		small_blocks[block_number] = NULL;
		break;
	case BLOCK_MEDIUM_SIZE:
		medium_blocks[block_number] = NULL;
		break;
	case BLOCK_LARGE_SIZE:
		large_blocks[block_number] = NULL;
		break;
	}
}

bool
check_double_free(void *addr, struct region *curr)
{
	if (!addr || curr->free) {
		printf("Invalid free: Already freed\n");
		return true;
	}
	return false;
}

bool
check_invalid_address(size_t size)
{
	if (size == 0) {
		printf("Invalid free: Invalid address\n");
		return true;
	}
	return false;
}

/// Public API of malloc library ///

void *
malloc(size_t size)
{
	if (size > BLOCK_LARGE_SIZE) {
		errno = ENOMEM;
		return NULL;
	}

	size_t requested_memory_size = size;
	size = ALIGN4(size);

	if (size < MINSIZE)
		size = MINSIZE;

	struct region *region = find_free_region(size);

	if (!region) {
		struct region *new_block = grow_heap(size);
		if (!new_block) {
			errno = ENOMEM;
			return NULL;
		}
		region = find_free_region(size);
	}

	// updates statistics
	amount_of_mallocs++;
	requested_memory += requested_memory_size;

	return REGION2PTR(region);
}


void
free(void *ptr)
{
	if (!ptr)
		return;

	struct region *curr = PTR2REGION(ptr);
	int block_number = 0;
	size_t size = size_of_block(curr, &block_number);
	void *addr = get_block_addr(size, block_number);

	if (check_double_free(addr, curr) || check_invalid_address(size))
		return;

	curr->free = true;
	amount_of_frees++;

	coalesce_regions(addr);

	if (all_regions_are_freed(addr)) {
		if (munmap(addr, size) == ERROR) {
			printfmt("munmap failed");
			exit(EXIT_FAILURE);
		}
		point_block_to_null(block_number, size);
	}
}

void *
calloc(size_t nmemb, size_t size)
{
	void *ptr = malloc(nmemb * size);
	if (!ptr)
		return NULL;
	return memset(ptr, 0, nmemb * size);
}


void *
realloc(void *ptr, size_t size)
{
	if (!ptr)
		return malloc(size);

	if (size == 0) {
		free(ptr);
		return NULL;
	}

	struct region *curr = PTR2REGION(ptr);

	int block_number = 0;
	size_t block_size = size_of_block(curr, &block_number);
	struct region *addr = get_block_addr(block_size, block_number);
	size_t old_size = curr->size;

	if (block_size == 0) {  // No fue alocado con malloc(3)
		errno = ENOMEM;
		return NULL;
	}

	size = ALIGN4(size);

	if (size > BLOCK_LARGE_SIZE) {
		errno = ENOMEM;
		return NULL;
	}

	if (size < MINSIZE) {
		size = MINSIZE;
	}

	if (size <= old_size) {
		if (old_size - size >= HEADER_SIZE + MINSIZE)
			return REGION2PTR(split_region(curr, size));

		return ptr;
	}

	if (curr->next && curr->next->free &&
	    (old_size + curr->next->size) >= size) {
		// Si el siguiente bloque esta libre y la suma de los tamaños es
		// mayor o igual al nuevo tamaño Se puede expandir el bloque
		// actual
		merge_regions(curr, curr->next);
		if (curr->size - size >= HEADER_SIZE + MINSIZE)
			return REGION2PTR(split_region(curr, size));

		return ptr;
	}

	struct region *prev = NULL;
	while (addr && addr != curr) {
		prev = addr;
		addr = addr->next;
	}

	if (prev && prev->free && (old_size + prev->size) >= size) {
		// Si el bloque anterior esta libre y la suma de los tamaños es
		// mayor o igual al nuevo tamaño Se puede expandir el bloque
		// anterior
		size_t prev_size = prev->size;
		memcpy(prev, curr, old_size);
		prev->size += prev_size;
		memset(curr, 0, old_size);
		if (prev->size - size >= HEADER_SIZE + MINSIZE)
			return REGION2PTR(split_region(prev, size));

		return REGION2PTR(prev);
	}

	void *new_ptr = malloc(size);
	if (!new_ptr) {
		return NULL;
	}

	memcpy(new_ptr, ptr, size);

	free(ptr);

	return new_ptr;
}
void
get_stats(struct malloc_stats *stats)
{
	stats->mallocs = amount_of_mallocs;
	stats->frees = amount_of_frees;
	stats->requested_memory = requested_memory;
	stats->amount_small_blocks = amount_small_blocks;
	stats->amount_medium_blocks = amount_medium_blocks;
	stats->amount_large_blocks = amount_large_blocks;
}