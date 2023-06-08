#include <stdio.h>
#include "io/io.h"
#include "system/system.h"
#include <stdlib.h>

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
  SystemState *state = malloc(sizeof(SystemState));
  initialiseSystemState(state, numberOfInstructions, instructions);
  bool executing = true;
  do {
    uint32_t instruction =
        readInstruction(state, (*state).programCounter);
    switch (execute(state, instruction)) {
      case 0:
        break;
      case 1: // Error message already printed
        return 1;
      case HALT:
        executing = false;
        break;
      default:
        fprintf(stderr, "Unknown error!");
        return UNKNOWN_ERROR;
    }
  } while (executing);
  outputToFile(state, argv[2]);
  free(state);
  return 0;
}

