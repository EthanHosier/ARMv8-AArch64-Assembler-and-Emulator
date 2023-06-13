#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include "../consts.h"
#include "../../memory/memory.h"

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
  Memory primaryMemory;
} SystemState;

extern void initialiseSystemState(SystemState *state, int, const uint32_t *);

extern void outputToFile(SystemState *state, char *);

extern int execute(SystemState *state, uint32_t instruction);

extern uint32_t readInstruction(SystemState *state, uint32_t);

#endif
