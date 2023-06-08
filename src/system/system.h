#ifndef ARM_SYSTEM_H
#define ARM_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include "../consts.h"

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
  uint8_t dataMemory[MEMORY_SIZE_BYTES];
  uint32_t instructionMemory[MAX_INSTRUCTIONS];
} SystemState;

extern void initialiseSystemState(SystemState *state, int, const uint32_t *);

extern void outputToFile(SystemState *state, char *, int);

extern int execute(SystemState *state, bool bits[], uint32_t instruction, int);

uint32_t readInstruction(SystemState *state, int numberOfInstructions);

#endif
