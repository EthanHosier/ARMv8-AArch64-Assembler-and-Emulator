#include <stdio.h>
#include "io/io.h"
#include "system/system.h"
#include <stdlib.h>
#include <inttypes.h>

static void getBits(uint32_t instruction, bool bits[]) {
  for (int i = 0; i < INSTRUCTION_SIZE_BITS; i++) {
    bits[i] = instruction & 1;
    instruction = instruction >> 1;
  }
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Invalid number of arguments!\n");
    return 1;
  }
  uint32_t instructions[MAX_INSTRUCTIONS];
  int numberOfInstructions = 0;
  if (readBinaryFile(argv[1], instructions, &numberOfInstructions)) {
    return 1;
  }
  // printInstructions(instructions, numberOfInstructions);
  SystemState *state = malloc(sizeof(SystemState));
  initialiseSystemState(state, numberOfInstructions, instructions);
  while (true) {
    fprintf(stdout, "PC: %"PRId64"\n", (*state).programCounter / 4);
    bool bits[INSTRUCTION_SIZE_BITS];
    // Most significant bit has the highest index
    uint32_t instruction =
        readInstruction(state, (*state).programCounter);
    getBits(instruction, bits);
    printInstruction(bits);
    switch (execute(state, bits, instruction)) {
      case 0:
        break;
      case 1: //Error message already printed
        return 1;
      case HALT:
        goto END_FDE_CYCLE;
      default:
        fprintf(stderr, "Unknown error!");
        return UNKNOWN_ERROR;
    }
  }
  END_FDE_CYCLE:
  outputToFile(state, argv[2]);
  free(state);
  return 0;
}

