#ifndef __ESP_HEAP_CAPS_H__
#define __ESP_HEAP_CAPS_H__

#include <stdint.h>
#include <stdio.h>

void *heap_caps_malloc(size_t size, uint32_t caps);

#endif