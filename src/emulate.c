#include <stdio.h>
#include "io/io.h"
#include "system/system.h"
#include <stdlib.h>

static void getBits(uint32_t instruction, bool bits[]) {
  for (int i = 0; i < INSTRUCTION_SIZE_BITS; i++) {
    bits[i] = instruction & 1;
    instruction = instruction >> 1;
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
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
  initialiseSystemState(state);
  while ((*state).programCounter < numberOfInstructions) {
    bool bits[INSTRUCTION_SIZE_BITS];
    // Most significant bit has the highest index
    getBits(instructions[(*state).programCounter], bits);
    printInstruction(bits);
    if (execute(state, bits)) {
      return 1;
    }
  }
  outputToFile(state);
  free(state);
  return 0;
}

