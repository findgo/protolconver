
#include <stdlib.h>

void *mo_malloc(size_t size)
{
    return malloc(size);
}

void mo_free(void *ptr)
{
    free(ptr);
}

void *mo_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

void *mo_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

