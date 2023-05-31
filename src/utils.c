//cute little utils module xx
#include <stdio.h>
#include "utils.h"

int readBinaryFile(char *filename, uint32_t *fileMemory, int MAX_WORDS) {
  size_t numWords = 0;

  // Open the binary file
  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    printf("Failed to open the file.\n");
    return 1;
  }

  // Read words from the binary file
  while (fread(&fileMemory[numWords], sizeof(uint32_t), 1, file) == 1) {
    numWords++;
    if (numWords >= MAX_WORDS) {
      printf("Maximum word limit reached.\n");
      break;
    }
  }

  fclose(file);
  return 0;
}