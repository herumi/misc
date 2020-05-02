#pragma once
#define _GNU_SOURCE
#include <stdlib.h>
int mie_posix_memalign(void **memptr, size_t alignment, size_t size);
void *mie_aligned_alloc(size_t alignment, size_t size);
void *mie_memalign(size_t alignment, size_t size);
void *mie_malloc(size_t size);
void mie_free(void *ptr);
void *mie_calloc(size_t n, size_t size);
void *mie_realloc(void *ptr, size_t size);
char *mie_strdup(const char *ptr);
size_t mie_malloc_usable_size(void *p);
void mie_init();
#define posix_memalign mie_posix_memalign
#define aligned_alloc mie_aligned_alloc
#define memalign mie_memalign
#define malloc mie_malloc
#define free mie_free
#define calloc mie_calloc
#define realloc mie_realloc
#define strdup mie_strdup
#define malloc_usable_size mie_malloc_usable_size

