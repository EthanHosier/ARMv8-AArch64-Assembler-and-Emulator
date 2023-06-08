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
  uint8_t primaryMemory[MEMORY_SIZE_BYTES];
} SystemState;

extern void initialiseSystemState(SystemState *state, int, const uint32_t *);

extern void outputToFile(SystemState *state, char *);

extern int execute(SystemState *state, uint32_t instruction);

extern uint32_t readInstruction(SystemState *state, uint32_t);

#endif
