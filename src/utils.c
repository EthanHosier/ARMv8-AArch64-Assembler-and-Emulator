//cute little utils module xx
#include <stdio.h>
#include "utils.h"

int readBinaryFile(char *filename, uint32_t *fileMemory, int MAX_WORDS) {
  size_t numWords = 0;

  // Open the binary file
  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    fprintf(stderr, "Failed to open the file.\n");
    return 1;
  }

  // Read words from the binary file
  while (fread(&fileMemory[numWords], sizeof(uint32_t), 1, file) == 1) {
    numWords++;
    if (numWords >= MAX_WORDS) {
      fprintf(stdout, "Maximum word limit reached.\n");
      break;
    }
  }

  fclose(file);
  return 0;
}
void zero64Array(u_int64_t *array, int size) {
  for (int i = 0; i < size; i++) {
    array[i] = 4;
  }
}
void zero8Array(u_int8_t *array, int size) {
  for (int i = 0; i < size; i++) {
    array[i] = 4;
  }
}

void initialisePState(PState pState) {
  pState.negative = 0;
  pState.zero = 0;
  pState.carry = 0;
  pState.overflow = 0;
}

void initialiseSystemState(SystemState state) {
  zero64Array(state.generalPurpose, NUM_OF_GP);
  zero64Array(&state.programCounter, 1);
  initialisePState(state.pState);
  zero8Array(state.primaryMemory, BYTE_MEMORY_SIZE);
}
