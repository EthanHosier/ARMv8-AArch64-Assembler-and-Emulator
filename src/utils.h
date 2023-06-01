#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "consts.h"

int readBinaryFile(char *filename, uint32_t *output, int *instructionCount);

typedef struct {
  bool negative;
  bool zero;
  bool carry;
  bool overflow;
} PState;
typedef struct {
  uint64_t generalPurpose[GENERAL_PURPOSE_REGISTERS];
  uint64_t programCounter;
  PState pState;
  uint8_t primaryMemory[MEMORY_SIZE_BYTES];
} SystemState;

void initialiseSystemState(SystemState *state);

void getBits(uint32_t instruction, bool *bits);

void printInstructions(uint32_t instructions[], int numberOfInstructions);

void printInstruction(bool bits[]);

#endif
