#include "esp_heap_caps.h"

void *heap_caps_malloc(size_t size, uint32_t caps)
{
    return malloc(size);
}