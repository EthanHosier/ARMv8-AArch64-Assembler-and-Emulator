// goofy little utils functions xx
#include <stdio.h>
#include "utils.h"
#include <inttypes.h>

int readBinaryFile(char filename[], uint32_t output[], int *instructionCount) {
  int numWords = 0;
  // Open the binary file
  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    fprintf(stderr, "Failed to open file \".%s\"!\n", filename);
    return 1;
  }

  // Read instructions from the binary file
  while (fread(&output[numWords], INSTRUCTION_SIZE_BITS / 8, 1, file) == 1) {
    numWords++;
    if (numWords >= MAX_INSTRUCTIONS) {
      fprintf(stderr, "Maximum instruction count reached!\n");
      return 1;
    }
  }
  *instructionCount = numWords;
  fclose(file);
  return 0;
}
void zero64Array(uint64_t array[], int size) {
  for (int i = 0; i < size; i++) {
    array[i] = 0;
  }
}
void zero8Array(uint8_t array[], int size) {
  for (int i = 0; i < size; i++) {
    array[i] = 0;
  }
}

void initialisePState(PState *pState) {
  (*pState).negative = 0;
  (*pState).zero = 0;
  (*pState).carry = 0;
  (*pState).overflow = 0;
}

void initialiseSystemState(SystemState *state) {
  zero64Array((*state).generalPurpose, GENERAL_PURPOSE_REGISTERS);
  zero64Array(&(*state).programCounter, 1);
  initialisePState(&(*state).pState);
  zero8Array((*state).primaryMemory, MEMORY_SIZE_BYTES);
}

void getBits(uint32_t instruction, bool bits[]) {
  for (int i = 0; i < INSTRUCTION_SIZE_BITS; i++) {
    bits[i] = instruction & 1;
    instruction = instruction >> 1;
  }
}

void printInstruction(bool bits[]) {
  for (int i = 0; i < INSTRUCTION_SIZE_BITS; i++) {
    fprintf(stdout, "%i", bits[INSTRUCTION_SIZE_BITS - i - 1]);
    if (i % 4 == 3) {
      fprintf(stdout, " ");
    }
  }
  fprintf(stdout, "\n");
}

void printInstructions(uint32_t instructions[], int numberOfInstructions) {
  for (int i = 0; i < numberOfInstructions; i++) {
    fprintf(stdout, "Instruction %i: %" PRIu32 "\n", i + 1, instructions[i]);
  }
}
