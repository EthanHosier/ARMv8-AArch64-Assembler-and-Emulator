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
} pState;
struct processor {
  uint64_t genPurpose[NUM_OF_GP];
  uint64_t programCounter;
  pState pState;
};

#endif