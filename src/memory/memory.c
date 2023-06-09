#include "memory.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#define store_general(bits)\
  uint##bits##_t mask = (1 << ((8 * i + 7) - (8 * i) + 1)) - 1;\
  mask = mask << i * 8;\
  storeByteUnifiedMemory(memory,base + i, (int8_t) ((val & mask) >> i * 8));

static uint8_t readByteUnifiedMemory(Memory *memory,
                                     uint32_t address) {
  return (*memory).primaryMemory[address];
}

static void storeByteUnifiedMemory(Memory *memory,
                                   uint32_t address,
                                   int8_t value) {
  (*memory).primaryMemory[address] = value;
}

uint64_t readNBytes(Memory *memory,
                    uint32_t address, int bytes) {
  assert(bytes <= 8 && bytes >= 0);
  uint64_t val = 0;
  for (int i = 0; i < bytes; i++) {
    val = val | (uint64_t) readByteUnifiedMemory(memory,
                                                 address + i)
        << i * 8;
  }
  return val;
}

void writeNBytes(Memory *memory, uint64_t val, uint32_t base, int bytes) {
  assert(bytes >= 0);
  for (int i = 0; i < bytes; i++) {
    switch (bytes) {
      case 1: {
        store_general(8)
        break;
      }
      case 2: {
        store_general(16)
        break;
      }
      case 4: {
        store_general(32)
        break;
      }
      case 8: {
        store_general(64)
        break;
      }
      default:
        fprintf(stderr,
                "writeNBytes only accepts `bytes` values of 0, 1, 2, 4, or 8!");
        exit(1);
    }
  }
}
