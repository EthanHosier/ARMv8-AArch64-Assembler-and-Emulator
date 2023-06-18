#include "io.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

static void printBinaryHelper(uint32_t val, FILE *fileOut) {
  uint32_t mask = 1;
  for (int i = 0; i < sizeof(uint32_t); i++) {
      mask <<= 8;
      for (int j= 0; j < 8; j++) {
          fprintf(fileOut, "%"PRId32, mask & val);
          mask >>= 1;
      }
      mask <<= 8;
  }
}

void printBinary(ArrayList *binaryLines, char *fileName) {
  FILE *fileOut = fopen(fileName, "wb");
  if (fileOut == NULL) {
    printf("Failed to open the file.\n");
      exit(EXIT_FAILURE);
  }

  for (int i = 0; i < binaryLines->size; i++) {
    uint32_t element = *(uint32_t*)get_ArrayList_element(binaryLines, i);
      printBinaryHelper(element, fileOut);
  }
  fclose(fileOut);
}

void readFileToArray(char *fileName, ArrayList *lines) {
  FILE *fileIn = fopen(fileName, "r");
  if (fileIn == NULL) {
    printf("Failed to open the file.\n");
    exit(EXIT_FAILURE);
  }
  char* buffer = malloc(256*sizeof (char));
  while (fgets(buffer, 256, fileIn) != NULL) {
    if(buffer[0]=='\n') continue;
    buffer[strlen(buffer)-1]='\0';
    add_ArrayList_element(lines, strdup(buffer)); // read from files
  }
  free(buffer);
  fclose(fileIn);
}
