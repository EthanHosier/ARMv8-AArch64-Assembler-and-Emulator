#include <stdio.h>
#include "utils.h"

int executeImmediateDP(SystemState *state, bool bits[]) {
  // Todo: Body
  fprintf(stdout, "Immediate DP Instruction\n");
  (*state).programCounter++;
  return 0;
}

int executeRegisterDP(SystemState *state, bool bits[]) {
  /*
   * "and x0 x0 x0" is the halt instruction - use premature `return` statement
   * after overriding the value of the PC. This will avoid the PC being
   * incremented at the end of this function.
  */
  // Todo: Body
  fprintf(stdout, "Register DP Instruction\n");
  (*state).programCounter++;
  return 0;
}
int executeSingleDataTransfer(SystemState *state, bool bits[]) {
  // Todo: Body
  fprintf(stdout, "Single Data Transfer Instruction\n");
  (*state).programCounter++;
  return 0;
}
int executeLoadLiteral(SystemState *state, bool bits[]) {
  // Todo: Body
  fprintf(stdout, "Load Literal Instruction\n");
  (*state).programCounter++;
  return 0;
}
int executeBranch(SystemState *state, bool bits[]) {
  // Todo: Body
  fprintf(stdout, "Branch Instruction\n");
  (*state).programCounter++;
  return 0;
}

int execute(SystemState *state, bool bits[]) { // Don't forget about `nop` !!
  if (bits[28] && !bits[27] && !bits[26]) { // DP (Immediate)
    return executeImmediateDP(state, bits);
  } else if (bits[27] && !bits[26] && bits[25]) { // DP (Register)
    return executeRegisterDP(state, bits);
  } else if (bits[31] && bits[29] && bits[28] && bits[27] && !bits[26]
      && !bits[25] && bits[24] && !bits[23]) { // Single Data Transfer
    return executeSingleDataTransfer(state, bits);
  } else if (!bits[31] && !bits[29] && bits[28] && bits[27] && !bits[26]
      && !bits[25] && !bits[24]) { // Load Literal
    return executeLoadLiteral(state, bits);
  } else if (!bits[29] && bits[28] && !bits[27] && bits[26]) { // Branch
    return executeBranch(state, bits);
  }
  fprintf(stderr, "Invalid instruction type!");
  return 1;
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
  SystemState state;
  initialiseSystemState(&state);
  while (state.programCounter < numberOfInstructions) {
    bool bits[INSTRUCTION_SIZE_BITS];
    // Most significant bit has the highest index
    getBits(instructions[state.programCounter], bits);
    printInstruction(bits);
    if (execute(&state, bits)) {
      return 1;
    }
  }
  return 0;
}