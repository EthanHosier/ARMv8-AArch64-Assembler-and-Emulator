#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "consts.h"

int readBinaryFile(char *filename, uint32_t *fileMemory, int MAX_WORDS);

typedef struct {
  bool negative;
  bool zero;
  bool carry;
  bool overflow;
} PState;
typedef struct {
  uint64_t generalPurpose[NUM_OF_GP];
  uint64_t programCounter;
  PState pState;
  uint8_t primaryMemory[BYTE_MEMORY_SIZE];
} SystemState;
void initialiseSystemState(SystemState *state);

#endif