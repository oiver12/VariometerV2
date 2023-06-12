#ifndef flashMemory_h
#define flashMemory_h

#include <stdint.h>
#include <stdbool.h>

void flashMemoryInit(void);

void flashMemoryWriteInt32(const char* name, int32_t value);
bool flashMemoryReadInt32(const char* name, int32_t *value);

#endif