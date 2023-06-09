#include "../consts.h"
#include <stdint.h>

#ifndef MEMORY_H
#define MEMORY_H

typedef struct {
  uint8_t primaryMemory[MEMORY_SIZE_BYTES];
} Memory;

extern uint64_t readNBytes(Memory *memory, uint32_t address, int bytes);
void writeNBytes(Memory *memory, uint64_t, uint32_t base, int bytes);

#endif
