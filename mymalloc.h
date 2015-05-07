#include <stdlib.h>
int mie_posix_memalign(void **memptr, size_t alignment, size_t size);
void *mie_aligned_alloc(size_t alignment, size_t size);
void *mie_malloc(size_t size);
void mie_free(void *ptr);
void *mie_calloc(size_t n, size_t size);
void *mie_realloc(void *ptr, size_t size);
char *mie_strdup(const char *ptr);
void mie_dstr();
